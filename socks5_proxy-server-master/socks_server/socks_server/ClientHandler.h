#pragma once
#include <qobject.h>
#include <QTcpSocket>
#include <QDataStream>


class ClientHandler: public QObject
{
	Q_OBJECT
public:
	ClientHandler(QTcpSocket * socket);
	~ClientHandler();
	void setHost(const QString &host) { this->host = host; };
	void setPort(quint16 _port) { port = _port; };

private:
	QTcpSocket *leftSock;
	QTcpSocket *rightSock;
	QString host;
	quint16 port;

	void startRedirect();

private slots:
	void slotErrorRight(QAbstractSocket::SocketError);
	void clientToserver();
	void clientToserver2();
	void writeToRight();
	void writeToLeft();
	void onConnected();
};
