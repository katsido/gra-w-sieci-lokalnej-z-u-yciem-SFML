/////23:30


#include <SFML/Graphics.hpp>
#include "SFML/Network.hpp"

#include <iostream>
//#include <windows.h>
#include <fstream>
#include <string.h>
#include <time.h> 

//#include <unistd.h>


#define MAX_GRACZY 4
#define CO_ILE_PRZESYLAC 300

int ilosc_graczy = 0;
using namespace sf;
using namespace std;
enum sssss { Wspolrzedne, Kula };

class do_watku {
public:
	int id;
	UdpSocket *socket;

};

class do_watku_rozsylanie {
public:
	int id;
	UdpSocket *socket;

};


class gracz {
public:
	int numer_id;

	string imie;
	int wynik;
	int miejsce;//w punktacji
	unsigned short port_z_ktorego_wysyla;
	unsigned short port_z_ktorego_odbiera;
	double wsp_x;//wsp sa podawane w uk³adzie lokalnym tz rog bia³ego prostok¹ta to punkt(0,0)
	double wsp_y;
	int pop_kierunek;
	double celownik;
	string ip_klieckie;

	//unsigned short port_na
};
gracz tablica[MAX_GRACZY];



string serverAddress;//127.0.0.1
string clientAddress[MAX_GRACZY];


sf::UdpSocket *socket = NULL;


sf::UdpSocket *socket_zero = NULL;
sf::UdpSocket *socket_jeden = NULL;
sf::UdpSocket *socket_dwa = NULL;
sf::UdpSocket *socket_trzy = NULL;


sf::UdpSocket *socket_rozsylajacy = NULL;

//sf::Thread thread(&watek_odbiera_wspolrzedne);


//void watek_odbiera_wspolrzedne(int , UdpSocket)//wszystkie wiadomosci od gracza o podanym id
void watek_odbiera_wspolrzedne(do_watku dane)//wszystkie wiadomosci od gracza o podanym id
{
	sf::Packet packet;
	int typ_danych;
	int typ_danych2;
	double x, x2, y, y2;
	double wsp_x;
	double wsp_y;
	int alfa;
	int punkty;
	int kierunek;
	double celownik;
	//packet << Kasia.wsp_x << Kasia.wsp_y << Kasia.wynik << Kasia.poprzedni_kierunek<<Kasia.celownik_z_kierunkiem;

	std::cout << "watek odbierajacy wspolrzedne dzia³a dla gracza o id " << dane.id <<"\t\t!!!!!"<< std::endl;
	dane.socket->setBlocking(true);
	string stringsss;

	sf::IpAddress serverAddress2(clientAddress[dane.id]);
	while (true)
	{


		

		dane.socket->receive(packet, serverAddress2, tablica[dane.id].port_z_ktorego_wysyla);
		if (packet >> typ_danych)
		{
			typ_danych2 = typ_danych;
			cout << "\t\t\todebrano typ danych = " << typ_danych2 << endl;
		}
		switch (typ_danych2)
		{
		case Wspolrzedne:
			if (packet >> x2 >> y2 >> punkty >> kierunek >> celownik) {
				tablica[dane.id].wsp_x = x2;
				tablica[dane.id].wsp_y = y2;
				tablica[dane.id].wynik = punkty;
				tablica[dane.id].pop_kierunek = kierunek;
				tablica[dane.id].celownik = celownik;
				cout << "dane odebrana przez watek o id=" << dane.id <<"\t"<< x2 <<"\t"<< y2 << "\t"<<punkty << endl;
			}
			break;

		case Kula:
			//jakiœ kod
			//double wsp_x;
			//double wsp_y;
			//int alfa;
			cout << "POCISK !!!!!" << endl;

			break;

			//...
		case 88:
			if (packet >> punkty) {
				tablica[dane.id].wynik = punkty;

			}
			break;

		default:
			cout << "cos siê popsu³o. Ojej" << endl;
			break;
		}


		packet.clear();

	}
}


void watek_rozsylania(do_watku_rozsylanie dane)//wszystkie wiadomosci od gracza o podanym id
{
	std::cout << "watek rozsylajacy dziala " << std::endl;
	sf::Packet packet;
	dane.socket->setBlocking(true);
	int a = 0;
	int b = 1;
	int c = 9;
	int typ_danych;


	while (true)
	{

		for (int i = 0; i < ilosc_graczy; i++)
		{
			
			packet << tablica[i].wsp_x << tablica[i].wsp_y << tablica[i].wynik << tablica[i].pop_kierunek << tablica[i].celownik;
			//packet <<typ_danych<< Kasia.wsp_x << Kasia.wsp_y << Kasia.wynik << Kasia.poprzedni_kierunek<<Kasia.celownik_z_kierunkiem;

		}
		for (int i = 0; i < ilosc_graczy; i++)
		{
             sf::IpAddress serverAddress2(clientAddress[i]);
			 dane.socket->send(packet, serverAddress2, tablica[i].port_z_ktorego_odbiera);
		}


		std::cout << "wyslano tablice" << std::endl;
		a++;
		b++;

		sf::sleep(sf::milliseconds(CO_ILE_PRZESYLAC));//0.3 sekuddy


		packet.clear();

	}
}




int main()
{

	//clock_t zalogowanie_pierwszego_gracza;
	Clock zegar;
	Time deltaZegar;
	float dT = 0.0f;

	srand(time(0));
	int ktora = rand() % 3 + 1;

	sf::IpAddress a8 = sf::IpAddress::getLocalAddress();
	serverAddress = a8.getLocalAddress().toString();
	cout << "adres serwera ip: " << serverAddress << endl;

	unsigned short udpPort = 0;
	unsigned short udpPortServer = 42042;
	unsigned short udpPortClient = 42043;



	sf::Packet packet;
	sf::Packet packet2;
	sf::Packet packet3;
	bool server = true;
	udpPort = udpPortServer;



	// NETWORKING
	if (socket == NULL) {
		socket = new sf::UdpSocket();
		if (socket->bind(udpPort) != sf::Socket::Done) {
			sf::err() << "Failed to create UDP socket.";
		}
		socket->setBlocking(false);
	}

	if (socket_jeden == NULL) {
		socket_jeden = new sf::UdpSocket();
		if (socket_jeden->bind(0) != sf::Socket::Done) {//0 jako pierwszy wolny
			sf::err() << "Failed to create UDP socket.";
		}
		socket_jeden->setBlocking(false);
	}

	if (socket_dwa == NULL) {
		socket_dwa = new sf::UdpSocket();
		if (socket_dwa->bind(0) != sf::Socket::Done) {//0 jako pierwszy wolny
			sf::err() << "Failed to create UDP socket.";
		}
		socket_dwa->setBlocking(false);
	}

	if (socket_trzy == NULL) {
		socket_trzy = new sf::UdpSocket();
		if (socket_trzy->bind(0) != sf::Socket::Done) {//0 jako pierwszy wolny
			sf::err() << "Failed to create UDP socket.";
		}
		socket_trzy->setBlocking(false);
	}

	if (socket_zero == NULL) {
		socket_zero = new sf::UdpSocket();
		if (socket_zero->bind(0) != sf::Socket::Done) {//0 jako pierwszy wolny
			sf::err() << "Failed to create UDP socket.";
		}
		socket_zero->setBlocking(false);
	}

	if (socket_rozsylajacy == NULL) {
		socket_rozsylajacy = new sf::UdpSocket();
		if (socket_rozsylajacy->bind(0) != sf::Socket::Done) {//0 jako pierwszy wolny
			sf::err() << "Failed to create UDP socket.";
		}
		socket_rozsylajacy->setBlocking(false);
	}

	cout << "SOCKETY NA SERWERZE" << endl;
	cout << "socket gracza 0 = " << socket_zero->getLocalPort() << endl;
	cout << "socket gracza 1 = " << socket_jeden->getLocalPort() << endl;
	cout << "socket gracza 2 = " << socket_dwa->getLocalPort() << endl;
	cout << "socket gracza 3 = " << socket_trzy->getLocalPort() << endl;
	cout << "socket rozsylajacy = " << socket_rozsylajacy->getLocalPort() << endl;



	//clock_t pomiar = clock();
	//time_t timer;
	string aaaaaaaaaaa;
	string imie;
	string ip_klient;
	zegar.restart();
	unsigned short udpPort_wysylanie;
	unsigned short udpPort_odbieranie;
	unsigned short przyporzadkowany_dla_gracza;

	while (deltaZegar.asSeconds() <= 20.0f)
	{


		sf::IpAddress servera(sf::IpAddress::Broadcast);
		socket->receive(packet, servera, udpPortClient);
		if (packet >> imie >> udpPort_odbieranie >> udpPort_wysylanie >> ip_klient) {
			aaaaaaaaaaa = imie;
			cout << "odebrano na serwerze" << aaaaaaaaaaa << endl;;
			tablica[ilosc_graczy].numer_id = ilosc_graczy;
			tablica[ilosc_graczy].imie = imie;
			tablica[ilosc_graczy].miejsce = ilosc_graczy + 1;
			tablica[ilosc_graczy].port_z_ktorego_odbiera = udpPort_odbieranie;
			tablica[ilosc_graczy].port_z_ktorego_wysyla = udpPort_wysylanie;
			tablica[ilosc_graczy].wynik = 0;
			tablica[ilosc_graczy].ip_klieckie = ip_klient;
			clientAddress[ilosc_graczy] = ip_klient;


			if (ilosc_graczy == 0)przyporzadkowany_dla_gracza = socket_zero->getLocalPort();
			else if (ilosc_graczy == 1)przyporzadkowany_dla_gracza = socket_jeden->getLocalPort();
			else if (ilosc_graczy == 2)przyporzadkowany_dla_gracza = socket_dwa->getLocalPort();
			else if (ilosc_graczy == 3)przyporzadkowany_dla_gracza = socket_trzy->getLocalPort();


			//zalogowanie_pierwszego_gracza=clock();
			ilosc_graczy++;
			if (ilosc_graczy == 1) {
				zegar.restart();
				cout << "zegar start" << endl;
			}
			deltaZegar = zegar.getElapsedTime();
			cout << deltaZegar.asSeconds() << endl;
			//udpSocket.send(packet, recipientAddress, recipientPort);
			//udpSocket.receive(packet, senderAddress, senderPort);


			packet2 << deltaZegar.asSeconds() << ilosc_graczy - 1 << ktora << przyporzadkowany_dla_gracza << socket_rozsylajacy->getLocalPort() ;


			sf::IpAddress servera(clientAddress[ilosc_graczy-1]);
			socket->send(packet2, servera, udpPortClient);//wyslanie, adres nie moj


		}
		packet2.clear();

		deltaZegar = zegar.getElapsedTime();

		//cout << deltaZegar.asSeconds() << endl;



	}

	cout << "zaczeto wysylac liste imion" << endl;
	cout << "jest" << ilosc_graczy << " graczy w grze" << endl;
	packet3 << ilosc_graczy;
	for (int i = 0; i < ilosc_graczy; i++)
	{
		packet3 << tablica[i].imie;
		cout << "imie = " << tablica[i].imie << endl;

	}
	socket_rozsylajacy->setBlocking(true);
	for (int i = 0; i < ilosc_graczy; i++)
	{
		sf::IpAddress idziwidzi(clientAddress[i]);
		socket_rozsylajacy->send(packet3, idziwidzi, tablica[i].port_z_ktorego_odbiera);
	}
	packet3.clear();
	cout << "skonczono wysylac liste imion" << endl;

	do_watku zero;
	zero.id = 0;
	zero.socket = socket_zero;

	do_watku jeden;
	jeden.id = 1;
	jeden.socket = socket_jeden;

	do_watku dwa;
	dwa.id = 2;
	dwa.socket = socket_dwa;

	do_watku trzy;
	trzy.id = 3;
	trzy.socket = socket_trzy;
	sf::Thread thread3(&watek_odbiera_wspolrzedne, trzy);
	sf::Thread thread2(&watek_odbiera_wspolrzedne, dwa);
	sf::Thread thread1(&watek_odbiera_wspolrzedne, jeden);
	sf::Thread thread0(&watek_odbiera_wspolrzedne, zero);

	do_watku_rozsylanie ggg;
	ggg.socket = socket_rozsylajacy;

	sf::Thread thread_rozsylajacy(&watek_rozsylania, ggg);

	switch (ilosc_graczy)
	{
	case 4:
		thread3.launch();
	case 3:
		thread2.launch();
	case 2:
		thread1.launch();
	case 1:
		thread0.launch();
	}

	thread_rozsylajacy.launch();


	while (true)//faza glowna gry
	{
		//cout << "glowna faza" << endl;



	}




	return 0;
}