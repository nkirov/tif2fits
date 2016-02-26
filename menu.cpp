#include <QDebug>
#include <QFileInfo>

#include "menu.h"
#include "conv.h"
 
Menu::Menu(QWidget *parent) : QDialog(parent)
{
qDebug() << "start Menu";
	ui.setupUi(this);
	connect(ui.convert, SIGNAL(clicked()), this, SLOT(convert()));
	connect(ui.list, SIGNAL(textChanged(QString)), this, SLOT(newlist(QString)));
	connect(ui.copyright, SIGNAL(textChanged(QString)), 
						this, SLOT(newcopyright(QString)));
	connect(ui.convert, SIGNAL(pressed()), this, SLOT(processing()));

	red.setColor(QPalette::Base,QColor(Qt::red));
	white.setColor(QPalette::Base,QColor(Qt::white));

	newlist(list());
	newcopyright(copyright());
qDebug() << "stop Menu";
}

QString Menu::list() const
{ 	return ui.list->text();		}

QString Menu::copyright() const
{ 	return ui.copyright->text();	}

int Menu::wedge() const // 0 - no, 1 - TG13, 2 - TG21s
{	
	int b = 0;
	if (ui.TG13->isChecked()) b = 1;
	else if (ui.TG21s->isChecked()) b = 2;
qDebug() << " wedge = " << b;
	return b;
}

bool Menu::bigimage() const
{	return ui.bigimage->isChecked();	}

bool Menu::twoplates() const
{	return ui.twoplates->isChecked();	}

bool Menu::rotate() const
{	return ui.rotate->isChecked();	}

bool Menu::flop() const
{	return ui.flop->isChecked();	}

bool Menu::flip() const
{	return ui.flip->isChecked();	}

bool Menu::listFiles(QStringList & listf)
{
qDebug() << "begin listFiles";
qDebug() << list();
	QFile f(list());
	if (!f.open(QFile::ReadOnly))
	{	
qDebug() << "ERROR 001";
		return false;
	}
	QString name;
	QTextStream text(&f);
	while (!text.atEnd())
	{	
		text >> name;
		if (!name.isEmpty()) listf << name;
	}
	f.close();
qDebug() << "end listFiles" << listf;	
	return true;
}

int Menu::quadratic() const
{	return ui.quadratic->currentIndex();   }

void Menu::convert()
{
	QStringList listfiles;
		if (!listFiles(listfiles))
		{
qDebug() << "ERROR 002";
		}

		int i = 0; 
		while (i<listfiles.size())
		{
			QString fullname = "../" + listfiles[i] + "/" + listfiles[i]; 
// da! ../POT080_000099/POT080_000099.tif
//
			QString fullname2;
			if (twoplates()) 
			{
				++i;
 				fullname2 = "../" + listfiles[i-1] + "/" + listfiles[i]; 
			}
qDebug() << fullname << fullname2;
			ui.convert->setText(listfiles[i]);
			ui.convert->repaint();
			Conv* c = new Conv(fullname, fullname2, this);
			delete c;
			if (ui.dotr->isChecked()) 
			{
				ui.convert->setText(listfiles[i] + "r");
				ui.convert->repaint();
				Conv* d = new Conv(fullname + "r", fullname2, this);   // for POT080!!! and ROZ200
				delete d;
			}
			++i;
		}
	ui.convert->setText("DONE!");
	ui.convert->setDisabled(true);
}

void Menu::newlist(QString s)
{
	QFileInfo f(s);
	if (f.isFile()) ui.list->setPalette(white);
	else ui.list->setPalette(red);
}
	
void Menu::newcopyright(QString s)
{
	QFileInfo f(s);
	if (f.isFile()) ui.copyright->setPalette(white);
	else ui.copyright->setPalette(red);
}

void Menu::processing()
{
	ui.convert->setText("Processing...");
	ui.convert->update();
}
