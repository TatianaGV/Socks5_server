#pragma once
#include <QTcpSocket>
#include <QTcpServer>
#include <QDataStream>
#include "QTime"

class SocksProxy: public QObject
{
	Q_OBJECT

private:
	QTcpServer *m_ptcpServer;

public:
	SocksProxy(quint16 port);

public slots:
	void slotNewConnection();

};
