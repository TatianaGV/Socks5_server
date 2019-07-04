#include "ClientHandler.h"
#include <QtEndian>
#include <QHostAddress>
#include <QFile>

#pragma pack(push,1)
struct socks5request1
{
	quint8 version = 5;
	quint8 methods_count;
	quint8 methods[1];
};

struct socks5answer1
{
	quint8 version = 5;
	quint8 methodOfAuth = 0;
};

struct socks5request2
{
	quint8 version = 5;
	quint8 command;
	quint8 reserv = 0;
	quint8 typeAddress;
	quint32 host;
	quint16 port;
};

struct socks5answer2
{
	quint8 version = 5;
	quint8 statusAnswer;
	quint8 reserv = 0;
	quint8 typeAddress;
	quint32 host;
	quint16 port;
};
#pragma pack(pop)

const quint16 DNS_name = 3;
const quint16 IPv6 = 4;
const quint16 IPv4 = 1;

const quint16 GSSAPI = 1;
const quint16 NamePass = 2;



ClientHandler::ClientHandler(QTcpSocket *socket)
{
	leftSock = socket;
	leftSock->setParent(this);
	connect(leftSock, &QTcpSocket::disconnected, this, &ClientHandler::deleteLater);
	connect(leftSock, &QTcpSocket::readyRead, this, &ClientHandler::clientToserver);
}

ClientHandler::~ClientHandler()
{
	QTextStream cout(stdout);
	cout << "kill" << endl;
}

//first request from client
void ClientHandler::clientToserver()
{
	QTextStream cout(stdout);
	bool noauth = true;
	QByteArray buff = leftSock->readAll();
	socks5request1* request = reinterpret_cast<socks5request1*>(buff.data());
	quint8 count = request->methods_count;
	request->methods[count];
	//if request require authentication
	for (int i = 0; i < count; ++i) {
		if ((request->methods[i] == GSSAPI) && (request->methods[i] == NamePass))
			noauth = false;
	}
	//if method of authentication is consist of null, we send answer to client
	if (noauth)
	{
		socks5answer1 answer;
		leftSock->write(reinterpret_cast<char*>(&answer), sizeof(answer));
	}
	//if method of authntification is not consist of null, we disconnect with client
	else
	{
		leftSock->disconnectFromHost();
	}
	
	disconnect(leftSock, &QTcpSocket::readyRead, this, &ClientHandler::clientToserver);
	connect(leftSock, &QTcpSocket::readyRead, this, &ClientHandler::clientToserver2);

}


//second request from client
void ClientHandler::clientToserver2()
{
	QTextStream cout(stdout);
	QByteArray buff = leftSock->readAll();
	socks5request2* request = reinterpret_cast<socks5request2*>(buff.data());
	quint8 typeAddr = request->typeAddress;
	//if type of address is Dns-name or IPv6, we send to client error "type of address is not support"
	if ((typeAddr == DNS_name)||(typeAddr == IPv6))
	{
		socks5answer2 answer;
		answer.statusAnswer = 8;
		answer.host = 0;
		answer.port = 0;
		answer.typeAddress = 0;
		leftSock->write(reinterpret_cast<char*>(&answer), sizeof(answer));
	}
	//if type of address is IPv4, we send to client that access is allowed
	if (typeAddr == IPv4)
	{
		setHost(QHostAddress(qFromBigEndian(request->host)).toString());
		setPort(qFromBigEndian(request->port));
		disconnect(leftSock, &QTcpSocket::readyRead, this, &ClientHandler::clientToserver2);

		rightSock = new QTcpSocket(this);
		connect(rightSock, &QTcpSocket::connected, this, &ClientHandler::onConnected);
		connect(rightSock, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &ClientHandler::slotErrorRight);
		rightSock->connectToHost(host, port);
	}
}

void ClientHandler::onConnected()
{
	socks5answer2 answer;
	answer.statusAnswer = 0;
	answer.typeAddress = 1;
	answer.host = 0;
	answer.port = 0;
	leftSock->write(reinterpret_cast<char*>(&answer), sizeof(answer));
	startRedirect();
}

//connect left and right socket
void ClientHandler::startRedirect()
{
	connect(leftSock, &QTcpSocket::readyRead, this, &ClientHandler::writeToRight);
	connect(rightSock, &QTcpSocket::readyRead, this, &ClientHandler::writeToLeft);
	if (leftSock->bytesAvailable())
	{
		emit leftSock->readyRead();
	}
	if (rightSock->bytesAvailable())
	{
		emit rightSock->readyRead();
	}
}

void ClientHandler::writeToRight()
{
	QByteArray buff = leftSock->readAll();
	QFile file("myfile.txt");
	if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
	{
		file.write(buff);
		file.close();
	}
	rightSock->write(buff);
}


void ClientHandler::writeToLeft()
{
	QByteArray buff = rightSock->readAll();
	QFile file("myfile1.txt");
	if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
	{
		file.write(buff);
		file.close();
	}
	leftSock->write(buff);
}


void ClientHandler::slotErrorRight(QAbstractSocket::SocketError err)
{
	QString strError =
		"Error: " + (err == QAbstractSocket::HostNotFoundError ?
			"The host or port has not found." :
			err == QAbstractSocket::RemoteHostClosedError ?
			"The remote host is closed." :
			err == QAbstractSocket::ConnectionRefusedError ?
			"The connection was refused." :
			QString(rightSock->errorString())
			);

	QTextStream cout(stdout);
	cout << strError << "\n";
	leftSock->disconnectFromHost();
	rightSock->disconnectFromHost();
}