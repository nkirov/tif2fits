#ifndef MENU_H
#define MENU_H

#include <QString>
#include <QStringList>
#include "ui_menu.h"

class Menu : public QDialog
{
	Q_OBJECT
	public:
		Menu(QWidget *parent = 0);
		QString list() const;
		QString copyright() const;
		int wedge() const;
		bool bigimage() const;
		bool twoplates() const;
		bool rotate() const;
        bool flop() const; bool flip() const;
		bool listFiles(QStringList &);
		int quadratic() const;
	private:
		Ui::menu ui;
		QPalette red, white;
	public slots:
		void convert();
		void newlist(QString);
		void newcopyright(QString);
		void processing();
};
#endif
