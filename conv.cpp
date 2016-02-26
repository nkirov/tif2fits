#include <QFileInfo>

#include "conv.h"

Conv::Conv(QString f, QString f2, const Menu *menu)
{
qDebug() << "start Conv";
	typef = 
 "biiiibiibssssssssdssdssdssdsissssssdsssddssssisdddsiddiiddssssss";
//1234567890123456789012345678901234567890123456789012345678901234567890	
	fname = f;
	f2name = f2;                   // non "" for 2 plates

	cut = menu->wedge();
	nameCo = menu->copyright();
	flopp = menu->flop(); flipp = menu->flip();
	t.start();   	// time

	if (!readTif()) { qDebug() << "ERROR 1";	return;	}

	if (menu->twoplates())
	{
		if (!readHdr000(f2name)) { qDebug() << "ERROR f2name 2";	return;	}
		if (!changeHdr()) { qDebug() << "ERROR 3";	return;	}
		hdr2list = hdrlist;
	}
	if (!readHdr000(fname)) { qDebug() << "ERROR 2";	return;	}
    if (!changeHdr()) { qDebug() << "ERROR 3";	return;	}
/*   ***********************************************************     */
	if (menu->twoplates())
	{				 // true for 2 plates in 1 tif + wedge
		if (!cutcutWriteFits()) { qDebug() << "ERROR cutcut 3";	return;	}
	}
	else if (cut > 0)       			// for images with wedge!!!
	{
		if (menu->bigimage()) 
		{				// OK for POT015 !! (big images) ROZ200 ????
			if (!cutWriteFits00()) { qDebug() << "ERROR 4";	return;	}
		}
		else
		{
			if (menu->quadratic() == 0)
			{					// OK for POT080 !!
				if (!cutWriteFits()) { qDebug() << "ERROR 5";	return;	}
			}
			else
			{					// for BUC016 !!
				if (!cutWriteFitsNonQuadratic(menu->quadratic())) 
						{ qDebug() << "ERROR 55555";	return;	}
			}
		}
	}
	else 					// for images without wedge!!!
	{					// Bamberg
            
//		flop();   	//  for Bamberg - HAR025B050263
		if (menu->flop()) flop(); if (menu->flip()) flip();
//		rotate();   	//  for Bamberg - BAM010C001397
		if (menu->rotate())
        {
            rotate();
            if (!changeHdr()) { qDebug() << "ERROR 3333";	return;	}
        }
        if (!writeHdr(fname)) { qDebug() << "ERROR 6";	return;	}
        convertHdr();
		swap();
		if (!writeFits()) {
qDebug() << "ERROR 7";	return;
		}
	}

	qint64 tms = t.elapsed();  // time
qDebug() << "Time: " << tms;
	QTime t1(0,0,0);
	t1 = t1.addMSecs(tms);
qDebug() << t1.toString(Qt::TextDate);
qDebug() << "stop Conv";
}

Conv::~Conv()
{
	delete [] image;
	delete [] hdr;
}
/*************************************************/

int Conv::wedge_width()
{
qDebug() << "begin Conv::wedge_width()"; 
qDebug() << hdrlist[51].mid(9,19) << hdrlist[51].mid(9,19).toInt(); // dpi/254000*15000
	int wedge_width = 15; // 20 mm
	if (cut == 1) 	wedge_width = 15; //15;  
	else 		wedge_width = 20;
	int w = hdrlist[51].mid(9,19).toInt()*wedge_width/25.4;
qDebug() << "end Conv::wedge_width()" << w; 
	return w;
}

void Conv::swap()
{
qDebug() << "begin::swap()";
	uint* num = new uint;
	char* ch = reinterpret_cast<char *>(num);
	char ch0;
	for(qint64 k = 0; k < imageSize; k++)
	{ 
		*num = image[k] - 32768; 

		ch0 = ch[0]; ch[0] = ch[1]; ch[1] = ch0;
		image[k] = *num;
	}
	delete num;
	
qDebug() << "end::swap()";
}

void Conv::swap000(uint* im, int imSize)
{
qDebug() << "begin::swap()";
	uint* num = new uint;
	char* ch = reinterpret_cast<char *>(num);
	char ch0;
	for(qint64 k = 0; k < imSize; k++)
	{ 
		*num = im[k] - 32768; 

		ch0 = ch[0]; ch[0] = ch[1]; ch[1] = ch0;
		im[k] = *num;
	}
	delete num;	
qDebug() << "end::swap()";
}
/*************************************************/
bool Conv::readTif()
{
qDebug() << "begin Conv::readTif";
	QFile f(fname + ".tif");
	if (!f.open(QFile::ReadOnly))
	{	
qDebug() << "ERROR 1003";
		return false;
	}

	char *ch;

	qint64* num4 = new qint64(0);
        ch = reinterpret_cast<char *>(num4);
	for (int i = 0; i < 2; i++)
	{	
		f.getChar(&ch[0]); f.getChar(&ch[1]); f.getChar(&ch[2]); f.getChar(&ch[3]);
qDebug() << "*num4=" << *num4;
	}

	image = new uint[*num4/2];
	ch = reinterpret_cast<char *>(image);
qDebug() << "f.pos" << f.pos() << "reading...";	
 	f.read(ch, *num4 - 8); // OK
qDebug() << "f.pos" << f.pos();

	uint* num = new uint;
	ch = reinterpret_cast<char *>(num);
	for (int i = 0; i < 6; i++)
	{	
		f.getChar(&ch[0]); f.getChar(&ch[1]);
qDebug() << *num;
	}
	naxis1 = *num;
	for (int i = 0; i < 6; i++)
	{	
		f.getChar(&ch[0]); f.getChar(&ch[1]);
qDebug() << i << "->" << *num;
	}
	naxis2 = *num;
	imageSize = naxis1*naxis2;

	QFileInfo fi(f);
qDebug() << fi.created() << fi.lastModified();
	scan_date = fi.lastModified();
	f.close();
	delete num4;
	delete num;
qDebug() << naxis1 << naxis2 << naxis1*naxis2*2;
qDebug() << "end Conv::readTif";
	return true;
}
/*************************************************/
// OK
bool Conv::readHdr000(QString hdrname)
{
qDebug() << "begin Conv::readHdr000";
	QFile f(hdrname + ".hdr");
qDebug() << hdrname + ".hdr";	
	if (!f.open(QFile::ReadOnly))
	{	
qDebug() << "ERROR file -- missing!";
		return false;
	}
	hdrlist.clear();
	QTextStream text(&f);
	while (!text.atEnd())
	{
		QString line = text.readLine();
		while (line.length() < 80) line.append(" ");
// qDebug() << line.length() << line;
		hdrlist << line; 
	}
	f.close();
// remove DIR and END
	hdrlist.removeLast(); 	hdrlist.removeLast(); 
qDebug() << "end Conv::readHdr000 hdrlist.size()=" << hdrlist.size();
	return true;
}

/****************************************************************/

void Conv::changeField(QString &field, int i)
{
	if (typef[i] == 'b'|| typef[i] == 'i') 
		while (field.length() < 19) field = " " + field;
	else if (typef[i] == 's')
	{
		if (field == "00:00:00") field = " ";

		while (field.length() < 16) field = field + " ";
		field = " '" + field + "'"; 
	}
	else if (typef[i] == 'd')
	{
		field = field.replace(",", ".");
		double d = field.toDouble();
		field = QString::number(d, 'E', 12);	
		field = " " + field;
	}
}

void Conv::format(int i)
{
	QString s = hdrlist[i];
	int pos1 = s.indexOf('=');
	int pos2 = s.indexOf('/');
	QString field = s.mid(pos1 + 1, pos2 - pos1 - 1);
//qDebug() << s << field;
	field = field.remove("'");
	while (!field.isEmpty() && field[0] == ' ') field = field.remove(0,1);
	while (!field.isEmpty() && field[field.length() - 1] == ' ') 
		field = field.remove(field.length() - 1,1);

	changeField(field, i);

//qDebug() << field; 
	field = field + " ";
	s = s.replace(pos1 + 1, pos2 - pos1 - 1, field);
	while (s.length() > 80) s = s.remove(s.length() - 1, 1);
	while (s.length() < 80) s = s + " ";
//qDebug() << s; 
	hdrlist[i] = s;
}

// number x in line number n (n=3,4)
void Conv::changeSize(int x, int n)
{
qDebug() << "begin Conv::changeSize " << x;
	QString s = hdrlist[n];
	int pos1 = s.indexOf('=');
	int pos2 = s.indexOf('/');
	QString field = QString::number(x) + " ";
	while (field.length() < 20) field = " " + field;
	s = s.replace(pos1 + 1, pos2 - pos1 - 1, field);
	hdrlist[n] = s;
qDebug() << "end Conv::changeSize"; 
}
	
bool Conv::changeHdr()
{
qDebug() << "begin Conv::changeHdr" << naxis1 << naxis2;
// change sizes
	changeSize(naxis1, 3);	changeSize(naxis2, 4);

// replace 10.DATE (-1)
qDebug() << hdrlist[9];
	hdrlist[9].replace(10, 20, "'" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
qDebug() << hdrlist[9];

// replace 59.DATE-SCN  (0), (14.FIELD -> 67)
qDebug() << hdrlist[58];
	hdrlist[58].replace(10, 20, "'" + scan_date.toString("yyyy-MM-dd hh:mm:ss"));
qDebug() << hdrlist[58];

	for (int i=0; i<hdrlist.size(); i++) format(i);

	if (!copyright()) return false;
	return true;
}

bool Conv::copyright()
{
// include 66. (copyright)
	QFile fc(nameCo);
	if (!fc.open(QFile::ReadOnly))
	{	
qDebug() << "ERROR 504";
		return false;
	}
	QTextStream com(&fc);
	while (!com.atEnd())
	{
		QString line = com.readLine();
		while (!line.isEmpty() && line[0] == ' ') line = line.remove(0,1);
		line = "        " + line;
		while (line.length() < 80) line.append(" ");
		hdrlist << line; 
	}
	fc.close();

	QString line = "END"; 
	if (hdrlist[hdrlist.size() - 1].left(3) != line)
	{
		while (line.length() < 80) line.append(" ");
		hdrlist << line;
	}
qDebug() << "end Conv::copyright hdrlist.size()=" << hdrlist.size();
	return true;
}

void Conv::convertHdr()
{
qDebug() << "begin Conv::convertHdr";
// concert to one string and char*
	QString allhdr = hdrlist.join("");
	hdrSize = allhdr.length();
	hdr = new char[hdrSize];
	for (int i=0; i < hdrSize; i++)
		hdr[i] = allhdr[i].toAscii();
qDebug() << "end Conv::convertHdr hdrSize=" << hdrSize;
}

/*************************************************/
// директно записване на image
bool Conv::writeFits()
{
qDebug() << "begin Conv::writeFits";
	QFile f(fname + ".fits");
	if (!f.open(QIODevice::WriteOnly))
	{	
qDebug() << "ERROR";
		return false;
	}

qDebug() << "f.pos = " << f.pos();
	f.write(hdr, hdrSize);
	char space = ' ';
	int rest = (2880 - f.pos()%2880);
	for (int i = 0; i < rest; i++) f.putChar(space);

	char* ch = reinterpret_cast<char *>(image);
qDebug() << "f.pos = " << f.pos();	
 	f.write(ch, imageSize*2); 
qDebug() << "f.pos =" << f.pos();

	char ch0 = static_cast<char>(0);
	rest = (2880 - f.pos()%2880);
	for (int i = 0; i < rest; i++) f.putChar(ch0);
qDebug() << "f.pos = " << f.pos();
qDebug() << "end Conv::writeFits";
	return true;
}

// директно записване на im - параметър на функцията
bool Conv::writeFits000(QString name, uint* im, int imSize)
{
qDebug() << "begin Conv::writeFits000";
	QFile f(name + ".fits");
qDebug() << name;	
	if (!f.open(QIODevice::WriteOnly))
	{	
qDebug() << "ERROR";
		return false;
	}

qDebug() << "f.pos = " << f.pos();
	f.write(hdr, hdrSize);
	char space = ' ';
	int rest = (2880 - f.pos()%2880);
	for (int i = 0; i < rest; i++) f.putChar(space);

	char* ch = reinterpret_cast<char *>(im);
qDebug() << "f.pos = " << f.pos();	
 	f.write(ch, imSize*2); 
qDebug() << "f.pos =" << f.pos();

	char ch0 = static_cast<char>(0);
	rest = (2880 - f.pos()%2880);
	for (int i = 0; i < rest; i++) f.putChar(ch0);
qDebug() << "f.pos = " << f.pos();
qDebug() << "end Conv::writeFits000";
	return true;
}

/*************************************************/
// рязане на клин и записване на двата файла (image0, image1)
// за квадратни плаки
bool Conv::cutWriteFits()
{
qDebug() << "begin Conv::cutWriteFits()"; 
	long X = naxis1, Y = naxis2;
qDebug() << "X=" << X << "  Y=" << Y << " X*Y=" << X*Y;
qDebug() << sizeof(int) << sizeof(long);
	uint *image0 = new uint[X*X];
qDebug() << "image0=" << image0;
	if (image0 == NULL)
	{
qDebug() << "ERROR image0 == NULL"; 
	}
	uint *image1;
	if (Y > X)
	{
		image1 = new uint[X*(Y-X)];
qDebug() << "image1=" << image1;
		if (image1 == NULL)
		{
qDebug() << "ERROR image1 == NULL"; 
		}
/***** OK!
		int k = 0;
		for (; k < X*X; k++) image0[k] = image[k];
		for (int k1 = 0; k < X*Y; k++, k1++) image1[k1] = image[k];
******/
// OK
		if (fname[fname.length() - 1] != 'r')
		{
			for (int i=0; i < X; i++)
			{
//qDebug() << i;
				for (int j=0; j < X; j++)
					image0[i*X + j] = image[(X-i-1)*X + X-j-1];
			}
		}
		else
		{
			for (int i=0; i < X; i++)
				for (int j=0; j < X; j++)
					image0[i*X + j] = image[(X-j-1)*X + i];
		}
//qDebug() << "!!!!!!!!!!!!!  1";
		int Z = Y - X;
		for (int i = X, k = 0; i < Y && k < Z; i++, k++)
			for (int j=0; j < X; j++)
		{
			image1[(Z-k-1)*X + X-j-1] = image[i*X + j];
		}	
	}
	else return false;
//qDebug() << "!!!!!!!!!!!!!  2";
	changeSize(X, 3); changeSize(X, 4);
	convertHdr();
//qDebug() << "!!!!!!!!!!!!!  3";
	if (!writeHdr(fname)) {
qDebug() << "ERROR 101";
	}
	swap000(image0, X*X);
	if (!writeFits000(fname, image0, X*X)) {
qDebug() << "ERROR 102";
	}

qDebug() << hdrlist[3];
	changeSize(X, 3);
qDebug() << hdrlist[3];
	changeSize(Y-X, 4);
	convertHdr();
/*	
	if (!writeHdr(fname + "w")) {
qDebug() << "ERROR 103";
	}
*/	
	swap000(image1, X*(Y-X));
	if (!writeFits000(fname + "w", image1, X*(Y-X))) {
qDebug() << "ERROR 104";
	}

	delete [] image0;
	delete [] image1;
qDebug() << "end Conv::cutWriteFits()"; 
	return true;
}

/*************************************************/
// рязане на клин и записване на двата файла (image0, image1)
// за правоъгълни (не квадратни) - от коя страна е клина:
// 1 -> x>y, down; 2 -> x<y, down; 3 -> x>y, right; 4 -> x<y, right
// работи!!!
bool Conv::cutWriteFitsNonQuadratic(int variant)
{
qDebug() << "begin Conv::cutWriteFitsNonQuadratic()" << variant; 
	int X, Y, x2, y2;
// naxis1 = X, naxis2 = Y - from <name>.tif file!!!!!	
	X = naxis1; Y = naxis2;
qDebug() << "variant = " << variant << " X=" << X << " Y=" << Y << " X*Y=" << X*Y;

	int wedge = wedge_width();
	if (variant == 1 || variant == 2) 
	{
		x2 = X; y2 = Y - wedge;
	}
	else
	{
		x2 = X - wedge; y2 = Y;
	}
qDebug() << "variant = " << variant << "x2=" << x2 << "y2=" << y2 << "x2*y2=" << x2*y2;

	uint *image0 = new uint[x2*y2];
qDebug() << "image0=" << image0;
	if (image0 == NULL)
	{
qDebug() << "ERROR image0 == NULL"; 
	}
	int i, j, k;
	for (i = 0; i < y2; i++)	
		for (j = 0; j < x2; j++)
			if (flopp) image0[(y2-i-1)*x2 + x2-j-1] = image[i*X + j];
				//	image0[i*x2 + j] = image[(y2-i-1)*x2 + x2-j-1]; 
			else image0[(y2-i-1)*x2 + j] = image[i*X + j];
				//	image0[i*x2 + j] = image[(y2-i-1)*X + j];
qDebug() << i << j << i*x2 + j << x2*y2 << i*X + j << X*Y;

qDebug() << "!!!!!!!!!!!!!  2";
	changeSize(x2, 3); changeSize(y2, 4);
	convertHdr();
qDebug() << "!!!!!!!!!!!!!  3";
	if (!writeHdr(fname)) {
qDebug() << "ERROR 101";
	}
	swap000(image0, x2*y2);
	if (!writeFits000(fname, image0, x2*y2)) {
qDebug() << "ERROR 102";
	}
	delete [] image0;
qDebug() << "...............";

	int sizeimage1 = (variant == 1 || variant == 2) ? wedge*X : wedge*Y;
	uint *image1;
	image1 = new uint[sizeimage1];
qDebug() << "image1=" << image1;
	if (image1 == NULL)
	{
qDebug() << "ERROR image1 == NULL"; 
	}
	
	if (variant == 1 || variant == 2)     // wedge down
		for (i = y2, k = 0; i < Y; i++, k++)
			for (j = 0; j < X; j++)
				image1[k*x2 + j] = image[i*X + j];
	else 					// wedge right
		for (i = 0; i < Y; i++)
			for (j = x2, k = 0; j < X; j++, k++)				
				image1[i*wedge + k] = image[i*X + j];
qDebug() << i << k << j << k*x2 + j << sizeimage1 << i*X + j << X*Y;

	if (variant == 1 || variant == 2)
	{	
		changeSize(X, 3); changeSize(wedge, 4);
	}
	else
	{
		changeSize(wedge, 3); changeSize(Y, 4);
	} 
	convertHdr();
	
	swap000(image1, sizeimage1);
	if (!writeFits000(fname + "w", image1, sizeimage1)) {
qDebug() << "ERROR 104";
	}

	delete [] image1;
qDebug() << "end Conv::cutWriteFitsNonQuadratic()"; 
	return true;
}

/*************************************************/
// рязане на клин, рязане на две и записване на трите файла (image01, image02, image1)
bool Conv::cutcutWriteFits()
{
qDebug() << "begin Conv::cutcutWriteFits()"; 
	int x = naxis1, y = naxis2;
qDebug() << "x=" << y << "  y=" << y;
qDebug() << hdrlist[51].mid(9,19) << hdrlist[51].mid(9,19).toInt(); // dpi/254000*15000
	int wedge = wedge_width(); // = hdrlist[51].mid(9,19).toInt()*150/254;
qDebug() << "wedge = " << wedge;	

	int x2 = x/2;
	int y2 = y - wedge;
	uint *image01 = new uint[x2*y2];
	uint *image02 = new uint[x2*y2];
qDebug() << "image02=" << image02;
	if (image02 == NULL)
	{
qDebug() << "ERROR image02 == NULL"; 
	}
	uint *image1 = new uint[x*wedge];
	
qDebug() << "image1=" << image1;
	if (image1 == NULL)
	{
qDebug() << "ERROR image1 == NULL"; 
	}
	for (int i=0; i < y2; i++)
		for (int j=0; j < x2; j++)
			image01[i*x2 + j] = image[i*x + j];

	for (int i=0; i < y2; i++)
		for (int j=x2; j < x; j++)
			image02[i*x2 + j - x2] = image[i*x + j];

	for (int i = y2, k = 0; i < y && k < wedge; i++, k++)
		for (int j=0; j < x; j++)
	{
		image1[(wedge-k-1)*x + x-j-1] = image[i*x + j];
	}	

//qDebug() << "!!!!!!!!!!!!!  2";             do tuk !!!!!!!!!!!!!!!!!!!!!!!!
	changeSize(x2, 3); changeSize(y2, 4);
	convertHdr();
//qDebug() << "!!!!!!!!!!!!!  3";
	if (!writeHdr(fname)) {
qDebug() << "ERROR 101";
	}
	swap000(image02, x2*y2);
	if (!writeFits000(fname, image02, x2*y2)) {
qDebug() << "ERROR 102";
	}

	QStringList hdr1list = hdrlist;
	hdrlist = hdr2list;
	changeSize(x2, 3); changeSize(y2, 4);
	convertHdr();
//qDebug() << "!!!!!!!!!!!!!  3";
	if (!writeHdr(f2name)) {
qDebug() << "ERROR cutcut 101";
	}
	swap000(image01, x2*y2);
	if (!writeFits000(f2name, image01, x2*y2)) {
qDebug() << "ERRO cutcut 102";
	}

qDebug() << hdrlist[3];
	changeSize(x, 3);
qDebug() << hdrlist[3];
	changeSize(wedge, 4);
	convertHdr();	
	swap000(image1, x*wedge);
	if (!writeFits000(f2name + "w", image1, x*wedge)) {
qDebug() << "ERROR 104";
	}

	hdrlist = hdr1list;
	changeSize(x, 3);
qDebug() << hdrlist[3];
	changeSize(wedge, 4);
	convertHdr();	
//	swap000(image1, x*wedge);
	if (!writeFits000(fname + "w", image1, x*wedge)) {
qDebug() << "ERROR cutcut 104";
	}

	delete [] image01; delete [] image02;
	delete [] image1;
qDebug() << "end Conv::cutcutWriteFits()"; 
	return true;
}

/**************  for big files!!  *****************/
// рязане на клин и записване на два файла - на плаката директно, на клина от image1
// за квадратни плаки
bool Conv::cutWriteFits00()
{
qDebug() << "begin Conv::cutWriteFits00()"; 
	int X = naxis1, Y = naxis2;
qDebug() << "X=" << X << "  Y=" << Y << "X*X=" << X*X;

	if (Y < X) return false;

	changeSize(X, 3); changeSize(X, 4);
	convertHdr();
	if (!writeHdr(fname)) {
qDebug() << "ERROR 101";
	}

	QFile f(fname + ".fits");
	if (!f.open(QIODevice::WriteOnly))
	{	
qDebug() << "ERROR";
		return false;
	}

qDebug() << "f.pos = " << f.pos();
	f.write(hdr, hdrSize);
	char space = ' ';
	int rest = (2880 - f.pos()%2880);
	for (int i = 0; i < rest; i++) f.putChar(space);

	uint* num = new uint;
	char* ch = reinterpret_cast<char *>(num);
	char ch0;
qDebug() << "Writing...";		
	for (int i=0; i < X; i++)
	{
		if (i%2880 == 0) 
qDebug() << i;
		for (int j=0; j < X; j++)
		{
//			image0[i*X + j] = image[(X-i-1)*X + X-j-1];
			*num = image[(X-i-1)*X + X-j-1] - 32768;
			ch0 = ch[0]; ch[0] = ch[1]; ch[1] = ch0;
			f.write(ch, 2);
		}
	}
	f.close();
//qDebug() << "!!!!!!!!!!!!!  1";

	int Z = Y - X;
	uint *image1 = new uint[X*Z];
qDebug() << "image1=" << image1;
	if (image1 == NULL)
	{
qDebug() << "ERROR image1 == NULL"; 
	}
	for (int i = X, k = 0; i < Y && k < Z; i++, k++)
		for (int j=0; j < X; j++)
//			image1[(Z-k-1)*X + X-j-1] = image[i*X + j];     //for POT080
			image1[(Z-k-1)*X + j] = image[i*X + j];	// for POT015
//qDebug() << "!!!!!!!!!!!!!  2";

	changeSize(X, 3); changeSize(Z, 4);
	convertHdr();
	
	swap000(image1, X*(Y-X));
	if (!writeFits000(fname + "w", image1, X*Z)) {
qDebug() << "ERROR 104";
	}

	delete [] image1;
	delete num;
qDebug() << "end Conv::cutWriteFits00()"; 
	return true;
}
/******************* не е изпробвана! ******************************/
bool Conv::readFits()
{
	QFile f(fname + "_im.fits");
	if (!f.open(QFile::ReadOnly))
	{	
qDebug() << "ERROR";
		return false;
	}

	hdrSize = 2880;
	char * hdr = new char[hdrSize];
	f.read(hdr, hdrSize);

	imageSize = f.size() - hdrSize; 
	image = new uint[imageSize/2];
	char * ch = reinterpret_cast<char *>(image);
	f.read(ch, imageSize*2);
	
	f.close();
	return true;
}
/*************************************************/
bool Conv::writeHdr(QString name)
{
qDebug() << "begin Conv::writeHdr " << name;
	QFile f(name + ".hdrf");
	if (!f.open(QFile::WriteOnly))
	{	
qDebug() << "ERROR";
		return false;
	}
	QTextStream text(&f);
	for (int i=0; i < hdrlist.size(); i++)
		text <<  hdrlist[i] << endl;
	f.close();
qDebug() << "end Conv::writeHdr";	
	return true;
}
/**************************************************************/

// for X = Y
void Conv::rotate()
{
qDebug() << "begin Conv::rotate()";
	int X = naxis1, Y = naxis2;
qDebug() << "X=" << X << "  Y=" << Y;
	uint *image0 = new uint[X*Y];
qDebug() << "image0=" << image0;
	if (image0 == NULL)
	{
qDebug() << "ERROR image0 == NULL"; 
	}
    for (int i = 0; i < Y; i++)
		for (int j = 0; j < X; j++)
			image0[j*Y + i] = image[i*X + j];

	delete [] image;
	image = image0;
    naxis1 = Y; naxis2 = X;
qDebug() << "end Conv::rotate()"; 
}

// for X != Y
void Conv::flop()
{
qDebug() << "begin Conv::flop()"; 
	int X = naxis1, Y = naxis2;
qDebug() << "X=" << X << "  Y=" << Y;
	uint *image0 = new uint[X*Y];
qDebug() << "image0=" << image0;
	if (image0 == NULL)
	{
qDebug() << "ERROR image0 == NULL"; 
	}
	for (int i=0; i < Y; i++)
	{
//qDebug() << i;
		for (int j=0; j < X; j++)
//			image0[i*X + j] = image[(Y-i-1)*X + X-j-1];
			image0[i*X + j] = image[i*X + X-j-1];
	}
	delete [] image;
	image = image0;
qDebug() << "end Conv::flop()"; 
}

// for X != Y
void Conv::flip()
{
qDebug() << "begin Conv::flip()";
	int X = naxis1, Y = naxis2;
qDebug() << "X=" << X << "  Y=" << Y;
	uint *image0 = new uint[X*Y];
qDebug() << "image0=" << image0;
	if (image0 == NULL)
	{
qDebug() << "ERROR image0 == NULL";
	}
	for (int i=0; i < Y; i++)
	{
        //qDebug() << i;
		for (int j=0; j < X; j++)
			image0[i*X + j] = image[(Y - i - 1)*X + j];
	}
	delete [] image;
	image = image0;
qDebug() << "end Conv::flip()"; 
}


