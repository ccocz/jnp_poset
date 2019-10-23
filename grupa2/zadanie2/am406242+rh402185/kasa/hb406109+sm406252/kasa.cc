#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <float.h>
#include <limits>

using namespace std;

#define stopName first
#define departureTime second
#define ticketPrice second.first
#define ticketTime second.second
#define MP make_pair

#define ticketType map<string, pair<float, unsigned>>
#define courseType map<int, vector< pair<string, unsigned>>>

#define ADD_COURSE 1
#define ADD_TICKET 2
#define TICKET_QUERY 3
#define ERROR 4

#define MINUTES_IN_AN_HOUR 60
#define INITIAL_VECTOR_CAPACITY 20

/*
 * newCourse dla poprawnych nazw przystankow podanych w
 * przystanki i poprawnych czasow odjazdu podanych w czasy_odjazdu
 * tworzy kurs o numerze nr_linii, w którym kolejnoœæ przystankow i
 * odjazdow jest taka sama jak w podanych tablicach.
 */
bool newCourse(courseType& courses, int courseNo, size_t stopsNo,
               string stops[], unsigned departureTimes[]) {
    vector< pair<string, unsigned> > course;

    // Sprawdzmy czy istanial wczesniej kurs o takim samym numerze
    if (courses.count(courseNo))
        return false;

    map <string, bool> areRepeating;

    for (size_t i = 0; i < stopsNo; i++) {

        // Sprawdzamy czy ktorys przystanek nie wystepuje dwa razy
        if (areRepeating.count(stops[i]) == 1)
            return false;

        areRepeating[stops[i]] = true;

        course.emplace_back(MP(stops[i], departureTimes[i]));

        // Sprawdzamy czy czasy odjazdu sa podane w kolejnosci rosnacej
        if (i > 0 && course[i].departureTime < course[i - 1].departureTime)
            return false;
    }

    courses[courseNo] = course;

    return true;
}

// Tworzy nowy bilet o podanej nazwie, cenie i czasie waznosci.
bool newTicket(ticketType& tickets, const string& name, float price, unsigned time) {
    if (tickets.count(name))
        return false;

    tickets[name] = MP(price, time);

    return true;
}

// areConnected sprawdza czy da sie dojechac z jednego przystanku na drugi
// jezdzac podanym kursem.
bool areConnected(courseType& courses, unsigned courseNo,
                  const string& stop1, const string& stop2) {
    vector < pair<string, unsigned> > course = courses[courseNo];

    bool p1 = false, p2 = false;

    for (size_t i = 0; i < course.size(); i++) {
        if (p1 && course[i].stopName == stop2)
            p2 = true;

        if (course[i].stopName == stop1)
            p1 = true;
    }

    return p1 && p2;
}

// arrivalTime zwraca godzine przyjazdu podanego kursu na podany przystanek,
// jezeli przystanek nie nalezy do kursu to zwraca 0.
unsigned arrivalTime(courseType& courses, unsigned courseNo,
                     const string& stop) {
    vector <pair<string, unsigned>> course = courses[courseNo];

    for (const pair<string, unsigned>& currentStop : course) {
        if (currentStop.stopName == stop)
            return currentStop.departureTime;
    }

    return 0;
}

void printWait(const string& stop) {
    cout <<":-( " << stop << endl;
}

void printNoTickets() {
    cout << ":-|" << endl;
}

void printTickets(vector<string>& tickets) {
    cout << "! ";

    for (size_t i = 0; i < tickets.size(); i++) {
        cout << tickets[i];
        if (i != tickets.size() - 1)
            cout << "; ";
    }

    cout << endl;
}

bool ticketQuery(size_t& ticketCount, courseType& courses, ticketType& tickets,
                 size_t stopsNo, string stops[], unsigned courseNo[]) {

    for (size_t i = 0; i < stopsNo - 1; i++) {
        if (!courses.count(courseNo[i]))
            return false;

        if (!areConnected(courses, courseNo[i], stops[i], stops[i + 1]))
            return false;
    }

    for (size_t i = 0; i < stopsNo - 2; i++) {
        unsigned aT1 = arrivalTime(courses, courseNo[i], stops[i + 1]),
                aT2 = arrivalTime(courses, courseNo[i + 1], stops[i + 1]);

        if (aT1 > aT2)
            return false;

        if (aT1 < aT2) {
            printWait(stops[i + 1]);
            return true;
        }
    }

    unsigned totalTime = arrivalTime(courses, courseNo[stopsNo - 2], stops[stopsNo - 1]) -
                         arrivalTime(courses, courseNo[0], stops[0]);

    vector <string> optTickets;
    auto optPrice = numeric_limits<float>::infinity();

    for (pair<string, pair<float, unsigned>> ticket1: tickets) {
        if (ticket1.ticketTime > totalTime && ticket1.ticketPrice <= optPrice) {
            optTickets.clear();
            optTickets.push_back(ticket1.first);
            optPrice = ticket1.ticketPrice;
        }
        for (pair<string, pair<float, unsigned>> ticket2: tickets) {
            if (ticket1.ticketTime + ticket2.ticketTime > totalTime &&
                ticket1.ticketPrice + ticket2.ticketPrice <= optPrice) {
                optTickets.clear();
                optTickets.push_back(ticket1.first);
                optTickets.push_back(ticket2.first);
                optPrice = ticket1.ticketPrice + ticket2.ticketPrice;
            }
            for (pair<string, pair<float, unsigned>> ticket3: tickets) {
                if (ticket1.ticketTime + ticket2.ticketTime + ticket3.ticketTime > totalTime &&
                    ticket1.ticketPrice + ticket2.ticketPrice + ticket3.ticketPrice <= optPrice) {
                    optTickets.clear();
                    optTickets.push_back(ticket1.first);
                    optTickets.push_back(ticket2.first);
                    optTickets.push_back(ticket3.first);
                    optPrice = ticket1.ticketPrice + ticket2.ticketPrice + ticket3.ticketPrice;
                }
            }
        }
    }

    if (optTickets.empty()) {
        printNoTickets();
        return true;
    }


    ticketCount += optTickets.size();
    printTickets(optTickets);

    return true;
}

// Wypisuje odpowiedni komunikat na standardowe wyjœcie diagnostyczne.
void error(const size_t& lineNo, const string& line) {
    cerr << "Error in line " << lineNo << ": " << line << endl;
}

// Sprawdza, czy znak jest liter¹ alfabetu angielskiego.
bool isLetter(char c) {
    return ('A' <= c && c <= 'Z') ||
           ('a' <= c && c <= 'z');
}

// Sprawdza, czy znak jest spacj¹.
bool isSpace(char c) {
    return c == ' ';
}

// Sprawdza, czy znak jest liter¹ alfabetu angielskiego lub spacj¹.
bool isLetterOrSpace(char c) {
    return isLetter(c) || c == ' ';
}

// Sprawdza, czy znak jest cyfr¹.
bool isDigit(char c) {
    return '0' <= c  && c <= '9';
}

// Sprawdza, czy znak jest zerem.
bool isZero(char c) {
    return c == '0';
}

// Sprawdza, czy indeks przekroczy³ podan¹ wartoœæ;
bool outOfBound(size_t i, size_t length) {
    return i >= length;
}

// Inkrementuje wartoœci zmiennych i oraz start
// w sposób powszechnie u¿ywany w programie
void iterateIAndStart(size_t& i, size_t& start) {
    start = i + 1;
    i += 2;
}

// Zwraca napis, który jest czêœci¹ innego napisu na indeksach [start, i - 1].
string getSegment(const string& line, const size_t& i, const size_t& start) {
    return line.substr(start, i - start);
}

// Wrzuca napis, który jest czêœci¹ innego napisu na indeksach [start, i - 1]
// na koniec podanego vectora.
void pushSegment(const string& line, const size_t& i,
                 const size_t& start, vector<string>& vecStr) {
    vecStr.push_back(getSegment(line, i, start));
}

// Wyodrêbnia nazwê biletu z linii wejœcia i wrzuca
// j¹ do vectora przechowuj¹cego fragmenty linii
bool extractTicketName(const string& line, size_t& i,
                       size_t& start, vector<string>& vecStr) {
    // Sprawdzamy, gdzie koñczy siê nazwa biletu i czy jest poprawna
    for (; !outOfBound(i, line.length()) && isLetterOrSpace(line[i]); i++);

    // Sprawdzamy, czy reszta linii jest potencjalnie poprawna
    if (outOfBound(i, line.length()) || !isDigit(line[i]))
        return false;

    pushSegment(line, i - 1, start, vecStr);

    start = i;

    return true;
}

// Sprawdza, czy cena biletu ma co najmniej dwa poprawne miejsca po przecinku
bool maybeTwoDecimal(const string& line, const size_t& i,
                     const size_t& start) {
    return !outOfBound(i, line.length()) &&
           isDigit(line[start]) &&
           isDigit(line[i]);
}

// Wyodrêbnia cenê biletu z linii wejœcia i wrzuca
// j¹ do vectora przechowuj¹cego fragmenty linii
bool extractPrice(const string& line, size_t& i,
                  size_t& start, vector<string>& vecStr) {
    string tmp;

    // Sprawdzamy, czy cena biletu jest potencjalnie poprawna
    if (!isDigit(line[i]))
        return false;

    i++;

    for (; !outOfBound(i, line.length()) && isDigit(line[i]); i++);

    // Sprawdzamy, czy kropka jest tam, gdzie powinna byæ
    if (outOfBound(i, line.length()) || line[i] != '.')
        return false;

    tmp = getSegment(line, i, start);

    iterateIAndStart(i, start);

    // Sprawdzamy, czy cena ma co najmniej dwa poprawne miejsca po przecinku
    if (!maybeTwoDecimal(line, i, start))
        return false;

    // Do³¹czamy czêœæ po przecinku
    tmp.append(getSegment(line, i + 1, start));

    i++;

    // Sprawdzamy, czy cena ma dok³adnie dwa miejsca po przecinku
    if (outOfBound(i, line.length()) || !isSpace(line[i]))
        return false;

    vecStr.push_back(tmp);

    i++;
    start = i;

    return true;
}

// Wyodrêbnia czas wa¿noœci biletu z linii wejœcia i wrzuca
// go do vectora przechowuj¹cego fragmenty linii.
bool extractTime(const string& line, size_t& i, size_t& start, vector<string>& vecStr) {
    // Sprawdzamy, czy czas biletu mo¿e byæ poprawny
    if (isZero(line[i]) || !isDigit(line[i]))
        return false;

    i++;

    for (; !outOfBound(i, line.length()) && isDigit(line[i]); i++);

    // Sprawdzamy, czy czas biletu rzeczywiœcie by³ poprawny
    if (i != line.length())
        return false;

    pushSegment(line, i, start, vecStr);

    return true;
}

// Sprawdza, czy znak jest dopuszczalnym znakiem nazwy przystanku.
bool isValidNameChar(char c) {
    return isLetter(c) || c == '^' || c == '_';
}

// Wyodrêbnia numer kursu z linii wejœcia i wrzuca
// go do vectora przechowuj¹cego fragmenty linii.
bool extractLineNumber(const string& line, vector<string>& vecStr,
                       size_t& i, size_t& start) {
    for (; i < line.length() && isDigit(line[i]); i++);

    if (outOfBound(i, line.length()) || !isSpace(line[i]))
        return false;

    pushSegment(line, i, start, vecStr);

    iterateIAndStart(i, start);

    return true;
}

// Sprawdza, czy godzina ma jedn¹ cyfrê
bool oneDigitHour(const string& line, const size_t& i, const size_t& start) {
    return line[i] == ':' && isDigit(line[start]);
}

// Sprawdza, czy godzina ma dwie cyfry
bool twoDigitHour(const string& line, const size_t& i, const size_t& start) {
    return isDigit(line[start]) && isDigit(line[i]) &&
           !outOfBound(i + 1, line.length()) && line[i + 1] == ':';
}

// Wyodrêbnia godzinê przyjazdu na przystanek i wrzuca
// j¹ do vectora przechowuj¹cego fragmenty linii.
bool extractHour(const string& line, vector<string>& vecStr,
                 size_t& i, size_t& start) {
    if (oneDigitHour(line, i, start)) {
        pushSegment(line, start + 1, start, vecStr);
    }
    else if (twoDigitHour(line, i, start)) {
        pushSegment(line, start + 2, start, vecStr);
        i++;
    }
    else {
        return false;
    }

    iterateIAndStart(i, start);

    return true;
}

// Sprawdza, czy fragment napisu line potencjalnie jest minutow¹ czêœci¹
// godziny
bool maybeMinutes(const string& line, const size_t& i, const size_t& start) {
    return outOfBound(start, line.length()) || outOfBound(i, line.length()) ||
           !isDigit(line[start]) || !isDigit(line[i]);
}

// Wyodrêbnia minutow¹ czêœæ godziny przyjazdu na przystanek i wrzuca
// go do vectora przechowuj¹cego fragmenty linii.
bool extractMinutes(const string& line, vector<string>& vecStr,
                    size_t& i, size_t& start) {
    if (maybeMinutes(line, i, start))
        return false;

    pushSegment(line, start + 2, start, vecStr);
    i++;

    return true;
}

// Sprawdza, czy podany czas przyjazdu jest dopuszczalny
bool checkIfValidTime(vector<string>& vecStr) {
    string hour, minutes;
    int minutesNum, hourNum;
    char hChar;

    hour = vecStr.at(vecStr.size() - 2);
    minutes = vecStr.at(vecStr.size() - 1);

    minutesNum = stoi(minutes);

    hChar = hour.at(0);

    if (hour.length() == 1) {
        if ('0' <= hChar && hChar <= '4') {
            return false;
        }
        else if (hChar == '5') {
            if ((0 <= minutesNum && minutesNum <= 54) || minutesNum > 59)
                return false;
        }
    }
    else { // hour.length() == 2
        if (hChar == '0')
            return false;

        hourNum = stoi(hour);

        if (hourNum > 21 || (hourNum == 21 && minutesNum > 21))
            return false;
    }

    return minutesNum <= 59;
}

// Wyodrêbnia nazwê przystanku i wrzuca
// j¹ do vectora przechowuj¹cego fragmenty linii.
bool extractStopName(const string& line, vector<string>& vecStr,
                     size_t& i, size_t& start) {
    if (outOfBound(i, line.length()) || !isSpace(line[i]))
        return false;

    start = ++i;

    for (; !outOfBound(i, line.length()) && isValidNameChar(line[i]); i++);

    // Poprawna nazwa przystanku na koñcu linii, ale tylko,
    // gdy przed tym przystankiem by³y jakieœ poprzednie
    if (outOfBound(i, line.length()) && !vecStr.empty()) {
        pushSegment(line, i, start, vecStr);
    }
    // Niepoprawna nazwa przystanku lub zapytanie sk³ada³o siê
    // tylko z nazwy przystanku
    else if (outOfBound(i, line.length()) || !isSpace(line[i])) {
        return false;
    }
    // Poprawna nazwa przystanku wewn¹trz linii
    else {
        pushSegment(line, i, start, vecStr);

        iterateIAndStart(i, start);

        if (outOfBound(i, line.length()))
            return false;
    }

    return true;
}

// Dzieli liniê z komend¹ dodania kursu na odpowiednie
// informacje i wrzuca je do vectora
bool splitAddCourse(const string& line, vector<string>& vecStr) {
    size_t i, start;
    bool ok;

    start = 0;
    i = 0;

    if (!extractLineNumber(line, vecStr, i, start))
        return false;

    ok = true;

    while (!outOfBound(i, line.length()) && ok) {
        ok = extractHour(line, vecStr, i, start);

        if (ok) {
            ok = extractMinutes(line, vecStr, i, start);

            if (ok) {
                ok = checkIfValidTime(vecStr);

                if (ok)
                    ok = extractStopName(line, vecStr, i, start);
            }
        }
    }

    return ok;
}

// Dzieli liniê wejœcia na poszczególne informacje i umieszcza
// je w vectorze vecStr.
bool splitAddTicket(const string& line, vector<string>& vecStr) {
    size_t i, start;

    start = 0;
    i = 0;

    return extractTicketName(line, i, start, vecStr) &&
           extractPrice(line, i, start, vecStr) &&
           extractTime(line, i, start, vecStr);

}

// Dzieli liniê z zapytaniem o bilet na odpowiednie
// informacje i wrzuca je do vectora
bool splitTicketQuery(const string& line, vector<string>& vecStr) {
    size_t i, start;
    bool ok;

    i = 1;

    ok = extractStopName(line, vecStr, i, start);

    // Dekrementacje w pêtli s¹ wymagane, by móc u¿yæ tych samych funkcji,
    // co przy parsowaniu komendy opisuj¹cej now¹ liniê
    while (!outOfBound(i, line.length()) && ok) {
        i--;
        ok = extractLineNumber(line, vecStr, i, start);

        if (ok) {
            i = --start;

            ok = extractStopName(line, vecStr, i, start);
        }
    }

    return ok;
}

// Wywo³uje odpowiedni¹ funkcjê do dzielenia linii na fragmenty
bool splitLine(const string& line, int cmdId, vector<string>& strVec) {
    bool ret;

    switch (cmdId) {
        case ADD_COURSE:
            ret = splitAddCourse(line, strVec);
            break;
        case ADD_TICKET:
            ret = splitAddTicket(line, strVec);
            break;
        case TICKET_QUERY:
            ret = splitTicketQuery(line, strVec);
            break;
        default:
            ret = false;
            break;
    }

    return ret;
}

// Na podstawie pierwszego znaku linii wejœcia
// rozpoznaje, jak¹ komendê ma reprezentowaæ
// ta linia
int recognizeId(string line) {
    char c;

    c = line[0];

    if (isDigit(c))
        return ADD_COURSE;
    else if (isLetterOrSpace(c))
        return ADD_TICKET;
    else if (c == '?')
        return TICKET_QUERY;
    else
        return ERROR;
}

// Konwertuje godzinê w formie tekstowej na liczbê minut
// od pó³nocy
unsigned timeStoU(const string& hour, const string& minutes) {
    return MINUTES_IN_AN_HOUR * stoul(hour) + stoul(minutes);
}

// Wype³nia odpowiednie tablice informacjami z vectora,
// które s¹ potem wykorzystywane w funkcji do dodawania kursu
void fillAddCourseArrays(const vector<string>& strVec,
                         unsigned departureTimes[], string stops[]) {
    for (size_t j = 0, i = 1; i < strVec.size(); j++, i += 3) {
        departureTimes[j] = timeStoU(strVec[i], strVec[i + 1]);
        stops[j] = strVec[i + 2];
    }
}

// Przygotowuje dane wczytane z wejœcia, by by³y w formie odpowiedniej
// dla funkcji newCourse
bool prepRunAddCourse(const vector<string>& strVec, courseType& courses) {
    size_t stopNumber;
    int lineNumber;

    lineNumber = stoi(strVec.front());
    // Wszystkie elementy vectora poza pierwszym s¹ pogrupowane
    // w 3-elementowe sekcje w formacie: godzina minuty nazwa_przystanku.
    stopNumber = (strVec.size() - 1) / 3;

    unsigned departureTimes[stopNumber];
    string stops[stopNumber];

    fillAddCourseArrays(strVec, departureTimes, stops);

    return newCourse(courses, lineNumber, stopNumber,
                     stops, departureTimes);
}

// Sprawdza, czy podana cena nie przekroczy maksymalnej wartoœci typu float
bool lessThanMax(const string& maxFloat, const string& price) {
    size_t i;

    // Porównujemy tylko czêœæ przed przecinkiem.
    if (price.length() - 2 < maxFloat.length())
        return true;

    if (price.length() - 2 > maxFloat.length())
        return false;

    // price.length() - 2 == maxFloat.length()
    for (i = 0; i < maxFloat.length() && price[i] == maxFloat[i]; i++);

    if (i < maxFloat.length())
        return price[i] < maxFloat[i];

    // i == maxFloat.length(), czyli czêœæ ca³kowita identyczna
    return price.back() == '0' && price[price.length() - 2] == '0';
}

// Przygotowuje dane wczytane z wejœcia, by by³y w formie odpowiedniej
// dla funkcji newTicket
bool prepRunAddTicket(const vector<string>& strVec, ticketType& tickets, const string& maxFloat) {
    string name;
    float price;
    unsigned duration;

    name = strVec[0];

    if (!lessThanMax(maxFloat, strVec[1]))
        return false;

    price = stof(strVec[1]);

    duration = stoul(strVec[2]);

    return newTicket(tickets, name, price, duration);
}

// Wype³nia odpowiednie tablice informacjami z vectora,
// które s¹ potem wykorzystywane w funkcji ticketQuery
void fillTicketQueryArrays(const vector<string>& strVec, string stops[],
                           unsigned lineNumbers[]) {
    for (size_t i = 0; i < strVec.size(); i++) {
        if (i % 2 == 0)
            stops[i / 2] = strVec[i];
        else
            lineNumbers[i / 2] = stoul(strVec[i]);
    }
}

// Przygotowuje dane wczytane z wejœcia, by by³y w formie odpowiedniej
// dla funkcji ticketQuery
bool prepRunTicketQuery(size_t& ticketCount, const vector<string>& strVec,
                        courseType& courses, ticketType& tickets) {
    size_t stopNumber;

    stopNumber = (strVec.size() - 1) / 2 + 1;

    unsigned lineNumbers[stopNumber - 1];
    string stops[stopNumber];

    stops[0] = strVec.front();

    fillTicketQueryArrays(strVec, stops, lineNumbers);

    return ticketQuery(ticketCount, courses, tickets, stopNumber, stops, lineNumbers);
}

// Wywo³uje odpowiedni¹ funkcjê, która parsuje liniê wejœcia i przygotowuje
// pozyskane z niej informacje w formie odpowiedniej dla danej funkcji
bool prepareAndRun(size_t& ticketCount, const vector<string>& strVec,
                   int cmdId, courseType& courses, ticketType& tickets,
                   const string& maxFloat) {
    bool ret;

    switch (cmdId) {
        case ADD_COURSE:
            ret = prepRunAddCourse(strVec, courses);
            break;
        case ADD_TICKET:
            ret = prepRunAddTicket(strVec, tickets, maxFloat);
            break;
        case TICKET_QUERY:
            ret = prepRunTicketQuery(ticketCount, strVec, courses, tickets);
            break;
        default:
            ret = false;
            break;
    }

    return ret;
}

// Wywo³uje funkcje, które parsuj¹ liniê wejœcia i przygotowuj¹
// pozyskane z niej informacje w formie odpowiedniej dla danej funkcji
bool interpretAndRunLine(size_t& ticketCount, const string& line,
                         courseType& courses, ticketType& tickets,
                         const string& maxFloat) {
    if (line.length() == 0)
        return true;

    int cmdId;

    cmdId = recognizeId(line);

    if (cmdId == ERROR)
        return false;

    vector<string> split;
    split.reserve(INITIAL_VECTOR_CAPACITY);

    if (!splitLine(line, cmdId, split))
        return false;

    return prepareAndRun(ticketCount, split, cmdId, courses, tickets, maxFloat);
}

// Wypisuje ³¹czn¹ liczbê biletów proponowanych we wszystkich
// odpowiedziach na pytanie
void printTicketCount(const size_t& ticketCount) {
    cout << ticketCount << endl;
}

// Przygotowuje napisow¹ reprezentacjê wartoœci FLT_MAX przed przecinkiem
string prepareMaxFloat() {
    string maxFloat;
    size_t dot;

    maxFloat = to_string(FLT_MAX);
    dot = maxFloat.find('.');

    return maxFloat.substr(0, dot);
}

int main() {
    string line, maxFloat;
    size_t counter, ticketCount;
    courseType courses;
    ticketType tickets;

    counter = 1;
    ticketCount = 0;
    maxFloat = prepareMaxFloat();

    while (getline(cin, line)) {
        if (!interpretAndRunLine(ticketCount, line, courses, tickets, maxFloat))
            error(counter, line);

        counter++;
    }

    printTicketCount(ticketCount);

    return 0;
}
