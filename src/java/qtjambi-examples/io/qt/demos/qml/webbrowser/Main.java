/****************************************************************************
**
** Copyright (C) 1992-2009 Nokia. All rights reserved.
** Copyright (C) 2009-2020 Dr. Peter Droste, Omix Visualization GmbH & Co. KG. All rights reserved.
**
** This file is part of Qt Jambi.
**
** ** $BEGIN_LICENSE$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $END_LICENSE$

**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
package io.qt.demos.qml.webbrowser;

import io.qt.examples.QtJambiExample;
import io.qt.core.QSize;
import io.qt.core.QUrl;
import io.qt.declarative.QDeclarativeError;
import io.qt.declarative.QDeclarativeView;
import io.qt.gui.QIcon;
import io.qt.widgets.QApplication;
import io.qt.widgets.QMainWindow;
import io.qt.widgets.QWidget;

@QtJambiExample(name = "Qml Webbrowser", canInstantiate = "call-static-method:checkQtDeclarativeSupport")
public class Main extends QMainWindow{
	
	public Main(QWidget parent) {
		super(parent);
		createView(this);
	}
	
	public static boolean checkQtDeclarativeSupport(){
		try {
            Class.forName("io.qt.declarative.QDeclarativeView");
            return true;
        } catch (Exception e) {
        }
        return false;
	}
	
	public void setSize(QSize size){
		this.resize(size);
	}

	public static void main(String args[])
	{
	    QApplication.initialize(args);
	    QWidget view = createView(null);
	    if (QApplication.arguments().contains("-maximize"))
	        view.showMaximized();
	    else if (QApplication.arguments().contains("-fullscreen"))
	        view.showFullScreen();
	    else
	        view.show();
	    // without deleting the view reference before execStatic() the application crashes at quit().
	    // this is because the view object and all child objects exist longer than the QApplication instance. 
	    view = null;
	    QApplication.execStatic();
	    QApplication.shutdown();
	}

	private static QWidget createView(QMainWindow parent){
	    io.qt.Utilities.loadQtLibrary("QtNetwork");
	    io.qt.Utilities.loadQtLibrary("QtXml");
		io.qt.Utilities.loadQtLibrary("QtWebKit");
		QDeclarativeView view = new QDeclarativeView(parent);
		view.engine().setOutputWarningsToStandardError(true);
		if(parent!=null){
			view.engine().quit.connect(parent, "close()");
			parent.setWindowTitle("Webbrowser");
			parent.setWindowIcon(new QIcon("classpath:/org/qtjambi/images/qtlogo-128.png"));
			parent.setCentralWidget(view);
			view.sceneResized.connect(parent, "setSize(QSize)");
		}else{
			view.engine().quit.connect(QApplication.instance(), "quit()");
		    view.setWindowTitle("Webbrowser");
		    view.setWindowIcon(new QIcon("classpath:/org/qtjambi/images/qtlogo-128.png"));
		}
	    view.setSource(new QUrl("classpath:/org/qtjambi/demos/qml/webbrowser/qml/webbrowser.qml"));
	    for(QDeclarativeError error : view.errors()){
	    	System.out.println(error);
	    }
	    return view;
	}
}