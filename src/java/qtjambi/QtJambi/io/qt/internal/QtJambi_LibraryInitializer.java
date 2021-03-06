/****************************************************************************
**
** Copyright (C) 1992-2009 Nokia. All rights reserved.
** Copyright (C) 2009-2020 Dr. Peter Droste, Omix Visualization GmbH & Co. KG. All rights reserved.
**
** This file is part of Qt Jambi.
**
** ** $BEGIN_LICENSE$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** $END_LICENSE$

**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
package io.qt.internal;

import static io.qt.internal.NativeLibraryManager.isAvailableLibrary;
import static io.qt.internal.NativeLibraryManager.loadLibrary;
import static io.qt.internal.NativeLibraryManager.loadQtCore;
import static io.qt.internal.NativeLibraryManager.loadQtJambiLibrary;
import static io.qt.internal.NativeLibraryManager.loadSystemLibraries;

import java.io.File;

class QtJambi_LibraryInitializer {
	
	private QtJambi_LibraryInitializer() {}
	
    static {
        try {
            loadSystemLibraries();
            if(isAvailableLibrary(null, "icudata", null, NativeLibraryManager.ICU_VERSION)){
            	loadLibrary(QtJambi_LibraryInitializer.class, null, "icudata", null, NativeLibraryManager.ICU_VERSION);
        	}
        	if(isAvailableLibrary(null, "icuuc", null, NativeLibraryManager.ICU_VERSION)){
        		loadLibrary(QtJambi_LibraryInitializer.class, null, "icuuc", null, NativeLibraryManager.ICU_VERSION);
        	}
        	if(isAvailableLibrary(null, "icui18n", null, NativeLibraryManager.ICU_VERSION)){
        		loadLibrary(QtJambi_LibraryInitializer.class, null, "icui18n", null, NativeLibraryManager.ICU_VERSION);
        	}
        	File library = loadQtCore();
        	loadQtJambiLibrary();
        	File binLibFile = library.getParentFile();
        	QtJambiInternal.setQtPrefix(binLibFile.getParentFile());
        	QtJambi_LibraryShutdown.initialize();
        } catch(RuntimeException | Error t) {
            throw t;
        } catch(Throwable t) {
            throw new RuntimeException(t);
        }
    }

    public static void init() {}
}
