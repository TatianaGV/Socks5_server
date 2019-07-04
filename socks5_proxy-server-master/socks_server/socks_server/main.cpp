#include <QtCore/QCoreApplication>
#include "socks_server.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	SocksProxy server(1234);

	return a.exec();
}
