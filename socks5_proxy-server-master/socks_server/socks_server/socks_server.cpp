#include "socks_server.h"
#include "ClientHandler.h"


SocksProxy::SocksProxy(quint16 port)
{
	m_ptcpServer = new QTcpServer();
	QTextStream cout(stdout);

	if (!m_ptcpServer->listen(QHostAddress::Any, port))
	{
		cout << QString("Unable to start the server: %1\n").arg(m_ptcpServer->errorString());
		m_ptcpServer->close();
		return;
	}
	connect(m_ptcpServer, &QTcpServer::newConnection, this, &SocksProxy::slotNewConnection);
	cout << QString("Server started at port %1!\n").arg(port);

}


void SocksProxy::slotNewConnection()
{
	QTcpSocket *pClientSocket = m_ptcpServer->nextPendingConnection();
	auto clienthandler = new ClientHandler(pClientSocket);
	QTextStream cout(stdout);
	cout << "New connection registred" << endl;
}
