#ifndef CONV_H
#define CONV_H

#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QDebug>
#include <QFile>

#include "menu.h"

class Conv 
{
	public:
		typedef short uint; 
		Conv(QString, QString, const Menu*);
		~Conv();
		bool readTif();

		bool readHdr000(QString);
		bool changeHdr();
		void convertHdr();
		bool writeHdr(QString);

		bool writeFits();
		bool writeFits000(QString, uint*, int);
		bool cutWriteFits();
		bool cutWriteFitsNonQuadratic(int); 
			// 0 -> x=y, down; 1 -> x>y, down; 2 -> x<y, down; 
			// 3 -> x>y, right; 4 -> x<y, right
		bool cutcutWriteFits(); // for 2 plates
		bool cutWriteFits00(); // for big files
		bool readFits();

	private:
		QString typef;
		uint * image;
		qint64 imageSize; // in uint
		char * hdr;
		qint64 hdrSize;   // in bytes
		QStringList hdrlist, hdr2list;
		int naxis1, naxis2;
		QString fname, f2name;
		QString nameCo;
		QTime t;
		QDateTime scan_date;
		int cut;    // 0 - nowedge, 1 - TG13, 2 - TG21s
		bool flopp, flipp;

		QDateTime dateTimeOBS, dateTimeUT;
		double exptime;
		QString sitelong;

		void rotate();
		void flop(); void flip();
		void swap();
		void swap000(uint*, int);
		int wedge_width();

		void format(int);
		void changeSize(int, int);
		void changeField(QString&, int);

		double julianDays(const QDateTime);
		QTime lst(QDateTime, QString);

		bool copyright();
};
#endif
