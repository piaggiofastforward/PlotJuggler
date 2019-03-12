#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QThread>
#include <QCommandLineParser>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QSettings>

QString getFunnySubtitle(){

    qsrand(time(nullptr));

    int n = qrand() % 20;
    QSettings settings;
    // do not repeat it twice in a row
    while( settings.value("previousFunnySubtitle").toInt() == n)
    {
        n = qrand() % 20;
    }
    settings.setValue("previousFunnySubtitle", n);

    switch(n)
    {
    case 0: return "PlotJuggler does it better";
    case 1: return "Talk is cheap, show me the data!";
    case 2: return "The visualization tool that you deserve";
    case 3: return "Who needs Matlab?";
    case 4: return "Changing the world, one plot at a time";
    case 5: return "\"Harry Plotter\" was also an option";
    case 6: return "I like the smell of plots in the morning";
    case 7: return "Timeseries, timeseries everywhere...";
    case 8: return "I didn't find a better name...";
    case 9: return "\"It won't take long to code that\"..\n"
                "Davide, 2014";
    case 10: return "Visualize data responsibly";
    case 11: return "How could you live without it?";
    case 12: return "This time you will find that nasty bug!";
    case 13: return "Now, with less bugs than usual!";
    case 14: return "You log them, I visualize them";
    case 15: return "The fancy timeseries visualization tool";
    case 16: return "Send me a PR with your splashscreen phrase!";

    default: return "I don't always visualize data,\n"
                    "but when I do, I use PlotJuggler";
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("IcarusTechnology");
    app.setApplicationName("PlotJuggler");

    // Load an application style
    QFile styleFile( "://style/stylesheet.qss" );
    styleFile.open( QFile::ReadOnly );

    // Apply the loaded stylesheet
    QString style( styleFile.readAll() );
    app.setStyleSheet( style );

    QString VERSION_STRING = QString("%1.%2.%3").
            arg(PJ_MAJOR_VERSION).
            arg(PJ_MINOR_VERSION).
            arg(PJ_PATCH_VERSION);

    app.setApplicationVersion(VERSION_STRING);

    QCommandLineParser parser;
    parser.setApplicationDescription("PlotJuggler: the time series visualization tool that you deserve ");
    parser.addVersionOption();
    parser.addHelpOption();

    QCommandLineOption nosplash_option(QStringList() << "n" << "nosplash",
                                       QCoreApplication::translate("main", "Don't display the splashscreen"));
    parser.addOption(nosplash_option);

    QCommandLineOption test_option(QStringList() << "t" << "test",
                                   QCoreApplication::translate("main", "Generate test curves at startup"));
    parser.addOption(test_option);

    QCommandLineOption loadfile_option(QStringList() << "d" << "datafile",
                                       QCoreApplication::translate("main", "Load a file containing data"),
                                       QCoreApplication::translate("main", "file") );
    parser.addOption(loadfile_option);

    QCommandLineOption config_option(QStringList() << "l" << "layout",
                                     QCoreApplication::translate("main", "Load a file containing the layout configuration"),
                                     QCoreApplication::translate("main", "file") );
    parser.addOption(config_option);

    QCommandLineOption buffersize_option(QStringList() << "buffer_size",
                                     QCoreApplication::translate("main", "Change the maximum size of the streaming buffer (minimum: 10 default: 60)"),
                                     QCoreApplication::translate("main", "seconds") );
    parser.addOption(buffersize_option);

    parser.process( *qApp );

    /*
     * You, fearless code reviewer, decided to start a journey into my source code.
     * For your bravery, you deserve to know the truth, no matter how hard it is to accept it.
     * The splashscreen is useless; not only it is useless, it will make your start-up
     * time slower by a couple of seconds for absolutely no reason.
     * But what are two seconds compared with the time that PlotJuggler will save you?
     * The splashscreen is the connection between me and my users, the glue that keeps
     * together our invisible relationship.
     * Now it is up to you to decide: you can block the splashscreen forever or not,
     * reject a message that brings a little of happiness into your day, spent analyzing data.
     * Please don't do it.
     */

    if( parser.isSet(nosplash_option) == false)
    // if(false) // if you uncomment this line, a kitten will die somewhere in the world.
    {
        QPixmap main_pixmap(":/splash/resources/splash_2.jpg");

        int font_id = QFontDatabase::addApplicationFont("://resources/DejaVuSans-ExtraLight.ttf");
        QString family = QFontDatabase::applicationFontFamilies(font_id).at(0);
        QFont font(family);
        font.setStyleStrategy(QFont::PreferAntialias);

        QPainter painter;
        painter.begin( &main_pixmap);
        painter.setPen(QColor(255, 255, 255));
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        QString subtitle = getFunnySubtitle();
        int font_size = 25;
        int text_width = main_pixmap.width() - 100;
        do{
            painter.setFont( QFont(family, font_size--) );
        }while(font_size > 20 && painter.fontMetrics().width(subtitle) > text_width );

        QPoint topleft(50, main_pixmap.height()-90);
        QSize rect_size(text_width,90);
        painter.drawText( QRect(topleft, rect_size),
                          Qt::AlignHCenter | Qt::AlignVCenter, subtitle );
        painter.end();

        QSplashScreen splash(main_pixmap);
        QDesktopWidget* desktop = QApplication::desktop();
        const int scrn = desktop->screenNumber(QCursor::pos());
        const QPoint currentDesktopsCenter = desktop->availableGeometry(scrn).center();
        splash.move(currentDesktopsCenter - splash.rect().center());

        splash.show();
        app.processEvents();
        splash.raise();

        const auto deadline = QDateTime::currentDateTime().addMSecs( 100*(30 + subtitle.size()*0.4) );

        MainWindow w( parser );
        while( QDateTime::currentDateTime() < deadline && !splash.isHidden() )
        {
            app.processEvents();
            QThread::msleep(100);
            splash.raise();
        }
        splash.close();
        w.show();
        return app.exec();
    }
    else{
        MainWindow w( parser );
        w.show();
        return app.exec();
    }
    return 0;
}
