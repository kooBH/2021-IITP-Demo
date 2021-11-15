#include <QApplication>
#include "app.h"

int main(int argc, char*argv[]){
	QCoreApplication::addLibraryPath(".");

	QApplication iitp_2021(argc, argv);

	app  main;
	main.show();
	
	return iitp_2021.exec();
}