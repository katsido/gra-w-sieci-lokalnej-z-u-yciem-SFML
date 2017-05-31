//godz01:22

#include <SFML/Graphics.hpp>
#include "SFML/Network.hpp"
#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <cstdlib>
#include <math.h>
//#include <unistd.h>

#include <windows.h>

using namespace sf;
using namespace std;

#define CZAS_OCZEKIWANIA_NA_START 20

#define SZEROKOSC_KLOCKA 28
#define WYSOKOSC_KLOCKA 28
#define ILE_KLOCKOW_POZIOM 64

#define PRZESUNIECIE_X_PLANSZY 10
#define PRZESUNIECIE_Y_PLANSZY 90
#define SZEROKOSC_RAMKI 10
#define SZEROKOSC_OKNA 800
#define WYSOKOSC_OKNA 600

#define PREDKOSC_KULI 150
#define PROMIEN_KULI 2

#define WSP_CHODZENIA 150
#define WSP_CELOWNIKA 80

#define WSP_PREDKOSCI_SKOKU 0.5		//predkosc poczatkowa pionowa
#define WSP_TEMPA_SKOKU 0.5			//i opadania

#define ILE_TRWA_PELNA_ANIMACJA 0.4

#define KARA_FALA -50

#define CO_ILE_PRZESYLAC 300

#define R 13 //promieñ brudu
#define r 4//oko brudu
#define PRAWO 1
#define LEWO -1

clock_t poczatek_duzego_okna_czas;
clock_t poprzedni_czas;
double elapsed_secs = 0.00;
double delta_time = 0.00;

int ile_graczy_w_grze;
bool okno_aktywne = true;

bool server;
unsigned short udpPort = 0;
unsigned short udpPortServer = 42042;
unsigned short udpPortClient = 42043;

unsigned short udpPort_odbieranie = 0;
unsigned short udpPort_wysylanie = 0;

unsigned short udpPort_wlasny_na_serwerze = 0;
unsigned short udpPort_rozsylanie = 0;

class paczka_wysylana_i_odbierana {
public:
	double wsp_x;
	double wsp_y;
	int poprzedni_kierunek;
	float celownik = 0; //kat w stopniach od poziomu okreslajacy kierunek strzelania
	double celownik_z_kierunkiem = 0;
	int wynik = 0;
};



paczka_wysylana_i_odbierana paczuszka;

paczka_wysylana_i_odbierana tablica_graczy[4];


std::string serverAddress = "127.0.0.1";
std::string clientAddress = "127.0.0.1";//ustalany w linijka 485
int id;


enum sssss { Wspolrzedne, Kula, Punkty };
enum Dir { Pusty, Klocek, Nagroda, Przeszkoda };
enum AAA { Brak, Wznoszenie, Opadanie };
enum bbb { Nie, Tak };

template < class T >
string to_string(T t)
{
	stringstream ss;
	ss << t;
	return ss.str();
}

void uaktualnij_czas() {
	clock_t pomiar = clock();


	elapsed_secs = double(pomiar - poprzedni_czas) / CLOCKS_PER_SEC;

	poprzedni_czas = pomiar;
	//cout <<"teraz:"<<pomiar-poczatek_duzego_okna_czas<<"\tdelta:"<< elapsed_secs <<endl;
	//printf("delta");

}



int pozycjeKlockow[100][100];
//clock_t poczatki_nagrod[100][100];

//x i y podajemy w ukladzie lokalnym
int czy_jest_blok(int x, int y) {
	int xx = x / SZEROKOSC_KLOCKA;
	int yy = y / WYSOKOSC_KLOCKA;

	//cout << "sprawdzam dla pola x= " << x << " y= " << y << " czy tablica dla " << xx << " " << yy << "jest zajeta" << endl;
	if (pozycjeKlockow[xx][yy] == 1)
		return 1;
	else
		return 0;
}

class kula {
public:
	bool istnienie = false;
	double wsp_x;
	double wsp_y;
	int alfa;

	void porusz_kula(float delta) {

		this->wsp_x = this->wsp_x + cos(this->alfa*3.14 / 180) *delta*PREDKOSC_KULI;
		this->wsp_y = this->wsp_y + sin(this->alfa*3.14 / 180) *delta*PREDKOSC_KULI;

	}
	void czy_napotkala_przeszkode() {
		if (czy_jest_blok(this->wsp_x, this->wsp_y) == 1 ||
			this->wsp_x < 0 ||
			this->wsp_x >= ILE_KLOCKOW_POZIOM*SZEROKOSC_KLOCKA ||
			this->wsp_y <= 0 ||
			this->wsp_y >= 485)
		{
			this->istnienie = false;
			this->wsp_x = 0;
			this->wsp_y = 0;
			this->alfa = 0;
		}
	}




};

class kula kuleczka;







class gracz {
public:
	string imie;
	int id;
	double wsp_x;//wsp sa podawane w uk³adzie lokalnym tz rog bia³ego prostok¹ta to punkt(0,0)
	double wsp_y;
	int poprzedni_kierunek = PRAWO;
	int wynik = 0;
	int zycia = 5;
	double v_gora;
	int skok = Brak;
	float celownik = 0; //kat w stopniach od poziomu okreslajacy kierunek strzelania
	double celownik_z_kierunkiem = 0;

	void przesun_w_bok(int kierunek, float delta) {
		if (kierunek == PRAWO)
		{
			this->poprzedni_kierunek = PRAWO;
			this->wsp_x += delta*WSP_CHODZENIA;
			cout << "prawo \t";
			this->celownik_z_kierunkiem = this->celownik;
			if (czy_jest_blok(this->wsp_x + 2 * R, this->wsp_y + R) == 1)//blokada w prawo
			{
				this->wsp_x = ((int)((this->wsp_x + 2 * R) / SZEROKOSC_KLOCKA))*SZEROKOSC_KLOCKA - 2 * R;
			}
			//cout << "1. celownik: " << Kasia.celownik << "\t z kierunkiem: " << Kasia.celownik_z_kierunkiem << endl;
		}
		else
		{
			this->poprzedni_kierunek = LEWO;
			this->wsp_x -= delta*WSP_CHODZENIA;
			cout << "lewo \t";
			this->celownik_z_kierunkiem = 180 - this->celownik;
			if (czy_jest_blok(this->wsp_x, this->wsp_y + R) == 1)//blokada w prawo
			{
				this->wsp_x = ((int)((this->wsp_x) / SZEROKOSC_KLOCKA))*SZEROKOSC_KLOCKA + SZEROKOSC_KLOCKA;
			}
			//cout << "2. celownik: " << Kasia.celownik << "\t z kierunkiem: " << Kasia.celownik_z_kierunkiem << endl;
		}
	}

	void zmien_celownik(float zmiana) {

		this->celownik += zmiana*WSP_CELOWNIKA;

		if (this->celownik >= 75)this->celownik = 75;
		if (this->celownik <= -75)this->celownik = -75;

		if (this->poprzedni_kierunek == PRAWO)
			this->celownik_z_kierunkiem = this->celownik;
		else//lewo
			this->celownik_z_kierunkiem = 180 - this->celownik;
		//cout << "3. celownik w dol: " << this->celownik << "\t z kierunkiem: " << this->celownik_z_kierunkiem << endl;
	}

	//ograniczenia aby kulka nie wyszla z planszy
	void ograniczenia() {

		if (this->wsp_x <= 0)this->wsp_x = 0;
		if (this->wsp_x >= ILE_KLOCKOW_POZIOM*SZEROKOSC_KLOCKA - 2 * R)this->wsp_x = ILE_KLOCKOW_POZIOM*SZEROKOSC_KLOCKA - 2 * R;
		if (this->wsp_y <= 0)this->wsp_y = 0;
		if (this->wsp_y >= WYSOKOSC_OKNA - PRZESUNIECIE_Y_PLANSZY - 2 * R)
		{
			this->zycia--;
			this->skok = Brak;
			this->wynik += KARA_FALA;
			this->wsp_y = 0;//respaw nad miejscem gdzie spadles
		}

	}

	void skok_ruch(float delta_T) {

		if (this->skok == Wznoszenie || this->skok == Opadanie)
		{
			if (this->v_gora > 0)
				this->skok = Opadanie;
			if (this->v_gora < 0)
				this->skok = Wznoszenie;



			this->v_gora += WSP_TEMPA_SKOKU*delta_T;
			//this->wsp_x += 0.9*this->poprzedni_kierunek;//0.9 wspolczynik jak daleko skaczemy
			this->wsp_y += this->v_gora;

			if (czy_jest_blok(this->wsp_x + R, this->wsp_y) == 1 && this->skok == Wznoszenie)//udoskonalic trzeba i to sporo
			{
				this->wsp_y = ((int)(this->wsp_y / WYSOKOSC_KLOCKA))*WYSOKOSC_KLOCKA + WYSOKOSC_KLOCKA;
				this->skok = Opadanie;
				this->v_gora = 0;//mucho³apka
				//cout << "x: "<<this->wsp_x<<" y: "<<this->wsp_y<<"\tv_pion: "<<this->v_gora<<endl;
			}

			if (czy_jest_blok(this->wsp_x + R, this->wsp_y + 2 * R) == 1 && this->skok == Opadanie)//wykrywa klocki od spodu
			{
				this->skok = Brak;
			}



		}
	}

	void czy_spadanie()
	{

		if (czy_jest_blok(this->wsp_x + R, this->wsp_y + 2 * R) == 0 && this->skok == Brak)//spadanie
		{


			this->skok = Opadanie;
			this->v_gora = 0;

			if (czy_jest_blok(this->wsp_x + 2 * R, this->wsp_y + 2 * R) == 0)//nie ma bloku po prawej
			{
				this->wsp_x = ((int)(this->wsp_x / SZEROKOSC_KLOCKA))*SZEROKOSC_KLOCKA + SZEROKOSC_KLOCKA;
				//cout << "nie ma po prawej"<<endl;
			}
			else
				this->wsp_x = ((int)(this->wsp_x) / SZEROKOSC_KLOCKA)*SZEROKOSC_KLOCKA + SZEROKOSC_KLOCKA - 2 * R;

		}
		/*else
		this->skok = Brak;*/
	}

	void skok_start() {
		if (this->skok == Brak)
		{
			this->skok = Wznoszenie;
			this->v_gora = -WSP_PREDKOSCI_SKOKU;
			cout << "skok \t";
		}
	}

	void strzela() {
		kuleczka.wsp_x = this->wsp_x + R;
		kuleczka.wsp_y = this->wsp_y + R;
		kuleczka.alfa = this->celownik_z_kierunkiem;
		kuleczka.istnienie = true;



	}

};




class gracz Kasia; //tak wiem Kasia to g³upia nazwa, ale nazwe gracz i player ju¿ ktoœ zastosowa³ do elementów planszy; 

void wczytaj_plansze(int ktora) {

	string nazwa = "assets/mapa" + to_string(ktora) + ".txt";
	char tab2[1024];
	strcpy_s(tab2, nazwa.c_str());

	FILE* f;						//wersja windows
	fopen_s(&f, tab2, "r");	//wersja windows
	//FILE *f = fopen("plik3.txt", "r"); //wersja Linux

	cout << "plik otwarty" << endl;


	for (int i = 0; i < 17; i++) {//ile linijek wczytac
		char tablica[100];
		cout << fgets(tablica, 1000, f);
		for (int j = 0; j < 64; j++) {//ile znakow z linijki
			if (tablica[j] == 'A') {
				pozycjeKlockow[j][i] = Klocek;
			}
		}
	}

	//pozycjeKlockow[0][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[1][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[2][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[3][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[4][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[5][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[5][11] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[6][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[7][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[8][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[9][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[10][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[10][11] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[11][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[12][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[13][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[14][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[15][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[15][11] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[16][10] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[17][10] = 1; //dodatkowy bloczek pomocniczy

	//pozycjeKlockow[17][19] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[17][18] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[17][17] = 1; //dodatkowy bloczek pomocniczy
	//pozycjeKlockow[17][16] = 1; //dodatkowy bloczek pomocniczy
	fclose(f);

}

gracz tablica[4];



class do_watku {
public:
	UdpSocket *socket;
};


void watek_wysylania(do_watku dane) {
	cout << "ruszyl watek wysylania" << endl;
	int typ_danych = Wspolrzedne;
	sf::IpAddress ttt(serverAddress);
	sf::Packet packet;
	while (true)
	{
		packet << typ_danych << Kasia.wsp_x << Kasia.wsp_y << Kasia.wynik << Kasia.poprzedni_kierunek << Kasia.celownik_z_kierunkiem;
		cout << "\t\t\tzaraz wysle" << endl;
		dane.socket->send(packet, ttt, udpPort_wlasny_na_serwerze);
		cout << typ_danych << "\t" << Kasia.wsp_x << "\t" << Kasia.wsp_y << "\t" << Kasia.wynik << "\t" << Kasia.poprzedni_kierunek << "\t" << Kasia.celownik_z_kierunkiem << endl;
		cout << "\t\t\twyslalem" << endl;
		packet.clear();
		sf::sleep(sf::milliseconds(CO_ILE_PRZESYLAC));  //0.3 sekuddy

	}

};



void watek_odbierania(do_watku dane) {
	cout << "ruszyl watek odbierania" << endl;
	Packet packet;
	sf::IpAddress serverAddress(serverAddress);
	dane.socket->setBlocking(true);
	int punkty, pop;
	double x, y, cel_z_kier;
	unsigned short port = udpPort_rozsylanie;
	while (true) {
		cout << "czekam na dane" << endl;
		dane.socket->receive(packet, serverAddress, port);
		cout << "odebralem cos do mnie dotarlo" << endl;
		for (int i = 0; i < ile_graczy_w_grze; i++)
			if (packet >> x >> y >> punkty >> pop >> cel_z_kier) {
				tablica_graczy[i].wsp_x = x;
				tablica_graczy[i].wsp_y = y;
				tablica_graczy[i].wynik = punkty;
				tablica_graczy[i].poprzedni_kierunek = pop;
				tablica_graczy[i].celownik_z_kierunkiem = cel_z_kier;
				cout << "odebralem poprawnie" << endl;
			}
			else {
				cout << "odebrano cos innego" << endl;
			}



			//packet << tablica[i].wsp_x << tablica[i].wsp_y << tablica[i].wynik << tablica[i].pop_kierunek << tablica[i].celownik;
			//udpSocket.receive(packet, senderAddress, senderPort);
			//udpSocket.send(packet, recipientAddress, recipientPort);
			packet.clear();
	}



};




int main()
{

	srand(time(0));
	enum Dir { Right, Left, Up, Down };
	sf::Vector2i zmienna(32, Left);


	string str;
	string text;
	int bedzie_gra = Tak;
	double czas_wyswietlany;
	double delta_suma = 0;
	double delta_suma2 = 0;
	double przesuniecie_planszy_dyn = 0;
	int bbb = 0;

	sf::Clock clock2;
	float dT = 0.0f;
	int n = 0;
	int nMax = 100;

	Clock zegar;//30 sekund do rozpoczecia
	float sss2;//czas otrzymany z serwera ile czasu juz uplyne³o
	string czas_do_rozpoczecia;
	int ktora;

	Font arial;

	sf::Packet packet;
	sf::Packet packet2;
	bool server = false;
	udpPort = udpPortClient;


	sf::UdpSocket *socket = NULL;//powitanie
	sf::UdpSocket *socket_odbieranie = NULL;
	sf::UdpSocket *socket_wysylanie = NULL;

	sf::IpAddress a8 = sf::IpAddress::getLocalAddress();
	clientAddress = a8.getLocalAddress().toString();
	cout << "adres clienta ip: " << clientAddress << endl;

	cout << "podaj adres ip serwera: " << endl;
	string gggg;
	cin >> gggg;
	cout << "\npodales adres" << gggg << endl;
	serverAddress = gggg;




	if (!arial.loadFromFile("assets/arial.ttf")) {
		cout << "brak pliku z czcionk¹";
		//Sleep(5000);
		//usleep(50000);
		return -1;
	}




	RenderWindow maleOkno(VideoMode(300, 230, 32), "Hello");

	RectangleShape rect(Vector2f(300, 50));
	rect.setFillColor(Color(204, 153, 153));
	rect.setPosition(0, 0);

	Text text3;
	text3.setString("Witaj graczu");
	text3.setCharacterSize(24);
	text3.setColor(Color(77, 77, 77));
	text3.setFont(arial);
	text3.setStyle(Text::Bold);
	text3.setPosition(70, 10);
	Text napis1;
	Text imie;
	Text napis2;
	string nazwa = "";

	napis1.setString("Podaj swoje imie i nacisnij ENTER");
	napis1.setCharacterSize(18);
	napis1.setColor(Color(77, 77, 77));
	napis1.setFont(arial);
	napis1.setStyle(Text::Bold);
	napis1.setPosition(5, 60);

	imie.setString(".....");
	imie.setCharacterSize(18);
	imie.setColor(Color(30, 30, 230));
	imie.setFont(arial);
	imie.setStyle(Text::Bold);
	imie.setPosition(5, 100);


	napis2.setString("a jezeli chcesz WYJSC\nnacisnij Escape");
	napis2.setCharacterSize(18);
	napis2.setColor(Color(77, 77, 77));
	napis2.setFont(arial);
	napis2.setStyle(Text::Bold);
	napis2.setPosition(5, 150);





	while (maleOkno.isOpen())
	{
		Event zdarzenie;
		while (maleOkno.pollEvent(zdarzenie))
		{
			if (zdarzenie.type == Event::Closed) {
				maleOkno.close();
				bedzie_gra = Nie;
			}

			if (zdarzenie.type == Event::KeyPressed && zdarzenie.key.code == Keyboard::Escape) {


				maleOkno.close();
				bedzie_gra = Nie;
			}
			if (zdarzenie.type == Event::KeyPressed && zdarzenie.key.code >= Keyboard::A&& zdarzenie.key.code <= Keyboard::Z) {

				char liczba = zdarzenie.key.code + 'A';
				nazwa = nazwa + liczba;
				imie.setString(nazwa);

			}
			if (zdarzenie.type == Event::KeyPressed && zdarzenie.key.code == Keyboard::Return) {
				maleOkno.close();
				bedzie_gra = Tak;

				//udpPort = udpPortClient;
				// NETWORKING
				if (socket == NULL) {
					socket = new sf::UdpSocket();
					if (socket->bind(udpPort) != sf::Socket::Done) {
						sf::err() << "Failed to create UDP socket.";
					}
					socket->setBlocking(true);
				}

				if (socket_wysylanie == NULL) {
					socket_wysylanie = new sf::UdpSocket();
					if (socket_wysylanie->bind(0) != sf::Socket::Done) {//0 jako pierwszy wolny
						sf::err() << "Failed to create UDP socket.";
					}
					socket_wysylanie->setBlocking(false);
				}

				if (socket_odbieranie == NULL) {
					socket_odbieranie = new sf::UdpSocket();
					if (socket_odbieranie->bind(0) != sf::Socket::Done) {
						sf::err() << "Failed to create UDP socket.";
					}
					socket_odbieranie->setBlocking(true);
				}

				//udpSocket.send(packet, recipientAddress, recipientPort);
				//udpSocket.receive(packet, senderAddress, senderPort);

				udpPort_odbieranie = socket_odbieranie->getLocalPort();
				udpPort_wysylanie = socket_wysylanie->getLocalPort();

				cout << "moje porty to:" << endl;
				cout << "\todbieram dane na = " << udpPort_odbieranie << endl;
				cout << "\twysylam dane z = " << udpPort_wysylanie << endl;


				packet << nazwa << udpPort_odbieranie << udpPort_wysylanie << clientAddress;

				sf::IpAddress lola(serverAddress);
				socket->send(packet, lola, udpPortServer);//wyslanie, adres nie moj,adres odbiorcy

				//sf::IpAddress lola(sf::IpAddress::Broadcast);			//sf::IpAddress clientAddress(serverAddress);
				socket->receive(packet2, lola, udpPortServer);

				float sss;
				Time start_gry;
				int aaa;
				string aaaaa;
				int ktor;
				Time deltaZegar;


				unsigned short eee;//porty ktore otrzymuje
				unsigned short ddd;


				if (packet2 >> sss >> aaa >> ktor >> eee >> ddd) {///sss-czas,aaa-imie gracza,ktor-kyora gra
					id = aaa;
					sss2 = sss;
					start_gry = seconds(sss);
					zegar.restart();
					ktora = ktor;

					udpPort_wlasny_na_serwerze = eee;
					udpPort_rozsylanie = ddd;


					cout << "odebrano " << aaaaa << "\t" << start_gry.asSeconds() << endl;
					cout << "odebrano numery portu dla mnie:" << endl;
					cout << "\tport na ktory bede wysylac = " << udpPort_wlasny_na_serwerze << endl;
					cout << "\tport ktory rozsyla do wszystkich = " << udpPort_rozsylanie << endl;

				}
				else
				{
					cout << "nic nie odebrano" << endl;
				}

				socket->unbind();

			}

		}

		maleOkno.clear(Color(204, 153, 204));
		maleOkno.draw(rect);
		maleOkno.draw(text3);
		maleOkno.draw(napis1);
		maleOkno.draw(napis2);
		maleOkno.draw(imie);
		maleOkno.display();
	}

	//oczekiwanie na pozostalych graczy
	if (bedzie_gra == Tak) {

		Text text3;
		text3.setString("Witaj " + nazwa);
		text3.setCharacterSize(24);
		text3.setColor(Color(77, 77, 77));
		text3.setFont(arial);
		text3.setStyle(Text::Bold);
		text3.setPosition(70, 10);
		Text napis1;
		Text imie;
		Text napis2;
		String nazwa = "";

		napis1.setString("Trwa oczekiwanie na \npozosta³ych graczy");
		napis1.setCharacterSize(18);
		napis1.setColor(Color(77, 77, 77));
		napis1.setFont(arial);
		napis1.setStyle(Text::Bold);
		napis1.setPosition(5, 30);

		imie.setString(".....");
		imie.setCharacterSize(18);
		imie.setColor(Color(30, 30, 230));
		imie.setFont(arial);
		imie.setStyle(Text::Bold);
		imie.setPosition(5, 150);


		napis2.setString("Graaa zacznie sie za sekund");
		napis2.setCharacterSize(18);
		napis2.setColor(Color(77, 77, 77));
		napis2.setFont(arial);
		napis2.setStyle(Text::Bold);
		napis2.setPosition(5, 100);

		float czas1;
		float czas2 = sss2;
		float czas_wynik = 0;
		double ddd;
		String czas_wynik2;



		RenderWindow maleOkno_oczekiwanie(VideoMode(300, 230, 32), "Czekam");
		while (maleOkno_oczekiwanie.isOpen() && czas_wynik <= CZAS_OCZEKIWANIA_NA_START)
		{
			Event zdarzenie;
			while (maleOkno_oczekiwanie.pollEvent(zdarzenie))
			{
				if (zdarzenie.type == Event::Closed) {
					maleOkno_oczekiwanie.close();
					bedzie_gra = Nie;
				}

				if (zdarzenie.type == Event::KeyPressed && zdarzenie.key.code == Keyboard::Escape) {
					maleOkno_oczekiwanie.close();
					bedzie_gra = Nie;
				}

				//if (zdarzenie.type == Event::KeyPressed && zdarzenie.key.code == Keyboard::Return) {
				//	maleOkno_oczekiwanie.close();
				//	bedzie_gra = Tak;
				//}


			}
			czas1 = zegar.getElapsedTime().asSeconds();
			czas_wynik = czas1 + czas2;
			ddd = ((double)CZAS_OCZEKIWANIA_NA_START - czas_wynik);
			czas_wynik2 = to_string(ddd);
			imie.setString("" + czas_wynik2);

			maleOkno_oczekiwanie.clear(Color(204, 153, 204));

			maleOkno_oczekiwanie.draw(text3);
			maleOkno_oczekiwanie.draw(napis1);
			maleOkno_oczekiwanie.draw(napis2);
			maleOkno_oczekiwanie.draw(imie);

			maleOkno_oczekiwanie.display();
		}

		cout << "okno sie zamknelo" << endl;



	}





	cout << "po nacisnieciu enter" << endl;
	int liczba;
	string imiono;
	Packet paczka;
	socket_odbieranie->setBlocking(true);

	sf::IpAddress clientAddress(serverAddress);
	cout << "czekam na liste imion" << endl;
	socket_odbieranie->receive(paczka, clientAddress, udpPort_rozsylanie);
	cout << "odbieranie zakonczone" << endl;
	if (paczka >> liczba) {
		ile_graczy_w_grze = liczba;
		cout << "w grze jest " << ile_graczy_w_grze << " zalogowanych graczy" << endl;
		for (int i = 0; i < ile_graczy_w_grze; i++)
		{
			if (paczka >> imiono) {
				tablica[i].imie = imiono;
				cout << i << " imie= " << imiono << endl;
			}
		}
		cout << "lista imion jest kompletna" << endl;

	}
	else
	{
		cout << "nic nie odebrano" << endl; ////////???????????? WCHOdzi tutaj. Czemu?
	}
	paczka.clear();


	for (int i = 0; i < ile_graczy_w_grze; i++)
	{
		cout << tablica[i].imie << endl;
	}


	cout << "\n\nURUCHAMIAMY WATKI\n" << endl;
	do_watku ggg;
	ggg.socket = socket_wysylanie;
	sf::Thread thread_wysylajacy(&watek_wysylania, ggg);
	thread_wysylajacy.launch();



	do_watku fff;
	fff.socket = socket_odbieranie;
	sf::Thread thread_odbierajacy(&watek_odbierania, fff);
	thread_odbierajacy.launch();



	//Duze okno
	RenderWindow oknoAplikacji(VideoMode(SZEROKOSC_OKNA, 600, 32), "Fantastyczna Gra");
	if (bedzie_gra == Tak) {

		//int ktora = 2;// rand() % 3 + 1;//jest losowana na serwerze

		wczytaj_plansze(ktora);
		string nazwa2 = "assets/klocek" + to_string(ktora) + ".png";
		string nazwa3 = "assets/background" + to_string(ktora) + ".png";



		Texture background;
		if (!background.loadFromFile(nazwa3)) {
			cout << "brak tekstury tla";
			Sleep(5000);
			//usleep(50000);
			return -1;
		}

		Texture textura;
		if (!textura.loadFromFile(nazwa2)) {
			cout << "brak terkstury klockow";
			Sleep(5000);
			//usleep(50000);
			return -1;
		}

		Texture pistolet;
		if (!pistolet.loadFromFile("assets/pistolet.png")) {
			cout << "brak terkstury pistolet";
			//Sleep(5000);
			//usleep(50000);
			return -1;
		}



		sf::Texture pTexture;
		sf::Sprite image;

		if (!pTexture.loadFromFile("assets/brud_32c.png"))
		{
			cout << "Failed to load brud";
			return 0;
		}
		image.setTexture(pTexture);


		sf::Texture pTextureM;
		sf::Sprite moneta;

		if (!pTextureM.loadFromFile("assets/brud_32.png"))
		{
			cout << "Failed to load moneta";
			return 0;
		}
		moneta.setTexture(pTextureM);


		sf::Texture pTextureS;
		sf::Sprite serce;

		if (!pTextureS.loadFromFile("assets/serca1.png"))
		{
			cout << "Failed to load serce";
			return 0;
		}
		serce.setTexture(pTextureS);


		sf::Texture pTextureF;
		sf::Sprite fale;

		if (!pTextureF.loadFromFile("assets/fale2.png"))
		{
			cout << "Failed to load fale";
			return 0;
		}
		fale.setTexture(pTextureF);

		//sf::Texture texture;
		//if (!texture.loadFromFile("cegly.png", sf::IntRect(10, 10, 10, 10)))
		//{
		//	// error...
		//}



		Kasia.imie = nazwa;//"Kasia"
		Kasia.wsp_x = 50 + id * 300;
		Kasia.wsp_y = 50;
		Kasia.wynik = 0;

		poprzedni_czas = clock();
		poczatek_duzego_okna_czas = clock();
		uaktualnij_czas;

		sf::Text text2;
		text2.setString("FPS: ---");
		text2.setFont(arial);
		text2.setPosition(600.0f, 0.0f);
		text2.setColor(sf::Color::Red);
		text2.setCharacterSize(15);


		RectangleShape pasek_gorny(Vector2f(SZEROKOSC_OKNA, 80));
		pasek_gorny.setFillColor(Color(204, 153, 204));

		RectangleShape pasek_dolny(Vector2f(SZEROKOSC_OKNA, 15));
		pasek_dolny.setPosition(0, 585);
		pasek_dolny.setFillColor(Color(204, 153, 204));

		RectangleShape plansza(Vector2f(ILE_KLOCKOW_POZIOM*SZEROKOSC_KLOCKA, WYSOKOSC_OKNA - PRZESUNIECIE_Y_PLANSZY));
		//plansza_z_ramka.setFillColor(Color::White);
		plansza.setTexture(&background);



		RectangleShape ramka(Vector2f(SZEROKOSC_OKNA - 2 * PRZESUNIECIE_X_PLANSZY, WYSOKOSC_OKNA));//dolna krawedz ramki poza oknem
		ramka.setFillColor(Color::Transparent);
		ramka.setPosition(PRZESUNIECIE_X_PLANSZY, PRZESUNIECIE_Y_PLANSZY);
		ramka.setOutlineThickness(SZEROKOSC_RAMKI);
		ramka.setOutlineColor(Color::Cyan);


		Text gracz_imie;
		gracz_imie.setString(Kasia.imie);
		gracz_imie.setCharacterSize(18);
		gracz_imie.setColor(Color(204, 153, 77));
		gracz_imie.setFont(arial);
		gracz_imie.setStyle(Text::Bold);
		gracz_imie.setPosition(20, 20);

		Text wynik;
		wynik.setString("wynik:");
		wynik.setCharacterSize(18);
		wynik.setColor(Color::Black);
		wynik.setFont(arial);
		wynik.setStyle(Text::Bold);
		wynik.setPosition(250, 20);


		Text czas;
		czas.setCharacterSize(18);
		czas.setColor(Color(204, 153, 77));
		czas.setFont(arial);
		czas.setStyle(Text::Bold);
		czas.setPosition(120, 20);


		RectangleShape wynik_tlo(Vector2f(100, 60));
		wynik_tlo.setFillColor(Color(255, 153 + 50, 204));
		wynik_tlo.setPosition(105, 10);

		RectangleShape player_tlo(Vector2f(100, 60));
		player_tlo.setFillColor(Color(255, 153, 204));
		player_tlo.setPosition(5, 10);


		CircleShape pocisk(PROMIEN_KULI);
		pocisk.setFillColor(Color::Black);

		//kulka, ktora jest graczaem
		CircleShape brud(R);
		brud.setFillColor(Color::Black);
		CircleShape oko_brud_L(r);
		oko_brud_L.setFillColor(Color::White);
		CircleShape oko_brud_P(r);
		oko_brud_P.setFillColor(Color::White);

		//napis nad graczem z imieniem
		Text imie;
		imie.setString(Kasia.imie);
		imie.setCharacterSize(16);
		imie.setColor(Color(204, 153, 77));
		imie.setFont(arial);
		imie.setStyle(Text::Bold);

		string staty = "";
		for (int i = 0; i < ile_graczy_w_grze; i++)
		{
			staty = staty + to_string(tablica[i].imie) + "\t" + to_string(tablica_graczy[i].wynik) + "\n";
		}

		Text statystyki;

		statystyki.setString("#idziWidzi");
		statystyki.setCharacterSize(16);
		statystyki.setColor(Color::Black);
		statystyki.setFont(arial);
		statystyki.setStyle(Text::Bold);
		statystyki.setPosition(600, 15);
		statystyki.setString(staty);




		RectangleShape blokPlanszy(Vector2f(SZEROKOSC_KLOCKA, WYSOKOSC_KLOCKA));//klocki do planszy
		blokPlanszy.setTexture(&textura);





		while (oknoAplikacji.isOpen())
		{

			Event zdarzenie;
			while (oknoAplikacji.pollEvent(zdarzenie))
			{
				if (zdarzenie.type == Event::Closed)


					oknoAplikacji.close();

				if (zdarzenie.type == Event::KeyPressed && zdarzenie.key.code == Keyboard::Escape)
					oknoAplikacji.close();

				if (zdarzenie.type == Event::KeyPressed && zdarzenie.key.code == Keyboard::Space && okno_aktywne == true)
				{
					Kasia.skok_start();
				}

				if (zdarzenie.type == Event::KeyPressed && zdarzenie.key.code == Keyboard::Return && okno_aktywne == true)
				{
					Kasia.strzela();
				}
				if (zdarzenie.type == Event::GainedFocus)
				{
					okno_aktywne = true;
				}
				if (zdarzenie.type == Event::LostFocus)
				{
					okno_aktywne = false;
				}

			}


			sf::Time deltaTime = clock2.restart();


			dT += deltaTime.asSeconds();
			n += 1;

			if (n >= nMax) {
				float fps = dT / nMax;
				string fpsString = to_string(1.0f / fps);
				text2.setString("FPS: " + fpsString);
				n = 0;
				dT = 0.0f;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && okno_aktywne == true) {
				Kasia.zmien_celownik(-deltaTime.asSeconds());
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && okno_aktywne == true) {
				Kasia.zmien_celownik(deltaTime.asSeconds());
				//Kasia.wsp_y += 100.0f * deltaTime.asSeconds();
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && okno_aktywne == true) {
				Kasia.przesun_w_bok(LEWO, deltaTime.asSeconds());
				zmienna.y = Left;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && okno_aktywne == true) {
				Kasia.przesun_w_bok(PRAWO, deltaTime.asSeconds());
				zmienna.y = Right;
				//Kasia.wsp_x += 100.0f * deltaTime.asSeconds();
			}



			Kasia.skok_ruch(deltaTime.asSeconds());
			Kasia.czy_spadanie();
			Kasia.ograniczenia();

			uaktualnij_czas();

			//cout << " obecnie jest " << Kasia.skok<<"\t";

			if (kuleczka.istnienie == 1)
			{
				kuleczka.porusz_kula(deltaTime.asSeconds());
				kuleczka.czy_napotkala_przeszkode();
			}

			if (Kasia.wsp_x < (SZEROKOSC_OKNA - 2 * SZEROKOSC_RAMKI) / 2)
				przesuniecie_planszy_dyn = 0;

			else if (Kasia.wsp_x >= ILE_KLOCKOW_POZIOM*SZEROKOSC_KLOCKA - (SZEROKOSC_OKNA - 2 * SZEROKOSC_RAMKI) / 2)
				przesuniecie_planszy_dyn = -(ILE_KLOCKOW_POZIOM*SZEROKOSC_KLOCKA - (SZEROKOSC_OKNA - 2 * SZEROKOSC_RAMKI));
			else
				przesuniecie_planszy_dyn = -Kasia.wsp_x + (SZEROKOSC_OKNA - 2 * SZEROKOSC_RAMKI) / 2;

			delta_suma += deltaTime.asSeconds();
			if (delta_suma >= ILE_TRWA_PELNA_ANIMACJA / 3) {
				zmienna.x++;
				if (zmienna.x * 32 >= pTexture.getSize().x)
					zmienna.x = 0;
				delta_suma = 0;
			}
			image.setTextureRect(sf::IntRect(zmienna.x * 32, zmienna.y * 32, 32, 32));


			delta_suma2 += deltaTime.asSeconds();
			if (delta_suma2 >= ILE_TRWA_PELNA_ANIMACJA / 4) {
				bbb++;
				if (bbb * 26 >= pTextureS.getSize().x)
					bbb = 0;
				delta_suma2 = 0;
			}
			serce.setTextureRect(sf::IntRect(bbb * 26, 0 * 26, 26, 26));
			fale.setTextureRect(sf::IntRect(bbb * 26, 0 * 26, 26, 40));

			str = "wynik:\n " + to_string(Kasia.wynik);
			wynik.setString(str);

			//linia celownik //musi zostac w tym miejscu bo innaczej jest œmiesznie
			RectangleShape celownik(sf::Vector2f(150, 1));
			celownik.setFillColor(Color::Red);
			celownik.rotate(Kasia.celownik_z_kierunkiem);

			RectangleShape bron(sf::Vector2f(pistolet.getSize().x, pistolet.getSize().y));
			bron.setTexture(&pistolet);
			bron.setOrigin(-R - 5, 2);
			bron.rotate(Kasia.celownik_z_kierunkiem);
			bron.setPosition(Kasia.wsp_x + R + PRZESUNIECIE_X_PLANSZY + przesuniecie_planszy_dyn, Kasia.wsp_y + R + PRZESUNIECIE_Y_PLANSZY);

			czas_wyswietlany = (double)(clock() - poczatek_duzego_okna_czas) / CLOCKS_PER_SEC;

			czas.setString("czas:\n" + to_string(czas_wyswietlany) + " s");

			pocisk.setPosition(kuleczka.wsp_x + PRZESUNIECIE_X_PLANSZY + przesuniecie_planszy_dyn, kuleczka.wsp_y + PRZESUNIECIE_Y_PLANSZY);


			image.setPosition(Kasia.wsp_x + PRZESUNIECIE_X_PLANSZY + przesuniecie_planszy_dyn - 3, Kasia.wsp_y + PRZESUNIECIE_Y_PLANSZY - 3);
			brud.setPosition(Kasia.wsp_x + PRZESUNIECIE_X_PLANSZY + przesuniecie_planszy_dyn, Kasia.wsp_y + PRZESUNIECIE_Y_PLANSZY);
			oko_brud_L.setPosition(Kasia.wsp_x + PRZESUNIECIE_X_PLANSZY + R - 2 * r + przesuniecie_planszy_dyn, Kasia.wsp_y + PRZESUNIECIE_Y_PLANSZY + R - r);
			oko_brud_P.setPosition(Kasia.wsp_x + PRZESUNIECIE_X_PLANSZY + R + przesuniecie_planszy_dyn, Kasia.wsp_y + PRZESUNIECIE_Y_PLANSZY + R - r);

			imie.setPosition(Kasia.wsp_x + PRZESUNIECIE_X_PLANSZY + przesuniecie_planszy_dyn, Kasia.wsp_y - 30 + PRZESUNIECIE_Y_PLANSZY);

			celownik.setPosition(Kasia.wsp_x + R + PRZESUNIECIE_X_PLANSZY + przesuniecie_planszy_dyn, Kasia.wsp_y + R + PRZESUNIECIE_Y_PLANSZY);

			plansza.setPosition(PRZESUNIECIE_X_PLANSZY + przesuniecie_planszy_dyn, PRZESUNIECIE_Y_PLANSZY);



			staty = "";
			for (int i = 0; i < ile_graczy_w_grze; i++)
			{
				staty = staty + to_string(tablica[i].imie) + "\t" + to_string(tablica_graczy[i].wynik) + "\n";
			}
			statystyki.setString(staty);



			//RectangleShape bloczek(Vector2f(SZEROKOSC_KLOCKA, WYSOKOSC_KLOCKA));//klocek roboczy czerwony
			//bloczek.setFillColor(Color::Red);
			//bloczek.setPosition(16 * 25 + PRZESUNIECIE_X_PLANSZY, 16 * 25 + PRZESUNIECIE_Y_PLANSZY);

			//RectangleShape bloczek2(Vector2f(SZEROKOSC_KLOCKA, WYSOKOSC_KLOCKA));//klocek roboczy czerwony
			//bloczek2.setFillColor(Color::Yellow);
			//bloczek2.setPosition(15 * 25 + PRZESUNIECIE_X_PLANSZY, 16 * 25 + PRZESUNIECIE_Y_PLANSZY);

			//RectangleShape bloczek3(Vector2f(SZEROKOSC_KLOCKA, WYSOKOSC_KLOCKA));//klocek roboczy czerwony
			//bloczek3.setFillColor(Color::Magenta);
			//bloczek3.setPosition(jeden * 25 + PRZESUNIECIE_X_PLANSZY, dwa * 25 + PRZESUNIECIE_Y_PLANSZY);

			oknoAplikacji.clear(Color::White);

			oknoAplikacji.draw(pasek_gorny);
			oknoAplikacji.draw(wynik_tlo);

			oknoAplikacji.draw(plansza);


			oknoAplikacji.draw(player_tlo);
			oknoAplikacji.draw(gracz_imie);

			oknoAplikacji.draw(czas);
			oknoAplikacji.draw(text2);
			oknoAplikacji.draw(wynik);
			for (int i = 0; i < Kasia.zycia; i++)
			{
				serce.setPosition(400 + i * 30, 50);
				oknoAplikacji.draw(serce);
			}





			for (int i = 0; i<100; i++) {
				for (int j = 0; j < 100; j++) {
					if (pozycjeKlockow[i][j] == 1) {
						blokPlanszy.setPosition(i*SZEROKOSC_KLOCKA + PRZESUNIECIE_X_PLANSZY + przesuniecie_planszy_dyn, j*WYSOKOSC_KLOCKA + PRZESUNIECIE_Y_PLANSZY);
						oknoAplikacji.draw(blokPlanszy);
					}
				}
			}

			//oknoAplikacji.draw(bloczek);
			//oknoAplikacji.draw(bloczek2);
			//oknoAplikacji.draw(bloczek3);


			imie.setString(Kasia.imie);
			oknoAplikacji.draw(brud);
			oknoAplikacji.draw(oko_brud_L);
			oknoAplikacji.draw(oko_brud_P);
			oknoAplikacji.draw(imie);



			oknoAplikacji.draw(celownik);



			oknoAplikacji.draw(ramka);





			//oknoAplikacji.draw(pasek_dolny);


			for (int i = 0; i < (SZEROKOSC_OKNA * 4 / pTextureF.getSize().x); i++)
			{
				fale.setPosition(0 + i * 26 + SZEROKOSC_RAMKI, WYSOKOSC_OKNA - pTextureF.getSize().y);
				oknoAplikacji.draw(fale);
			}

			if (kuleczka.istnienie == true)
				oknoAplikacji.draw(pocisk);

			imie.setString(Kasia.imie);
			oknoAplikacji.draw(image);
			oknoAplikacji.draw(bron);


			//rysowanie wszystkich graczy eksperymentalne
			for (int i = 0; i < ile_graczy_w_grze; i++) {
				if (i != id) {
					image.setPosition(tablica_graczy[i].wsp_x + PRZESUNIECIE_X_PLANSZY + przesuniecie_planszy_dyn - 3, tablica_graczy[i].wsp_y + PRZESUNIECIE_Y_PLANSZY - 3);
					imie.setPosition(tablica_graczy[i].wsp_x + PRZESUNIECIE_X_PLANSZY + przesuniecie_planszy_dyn, tablica_graczy[i].wsp_y - 30 + PRZESUNIECIE_Y_PLANSZY);
					imie.setString(tablica[i].imie);
					oknoAplikacji.draw(image);
					oknoAplikacji.draw(imie);
				}
			}


			oknoAplikacji.draw(statystyki);
			oknoAplikacji.draw(serce);
			oknoAplikacji.display();

		}
	}

	return 0;
}