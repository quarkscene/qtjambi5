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
package io.qt.autotests;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.IOException;
import java.io.Serializable;
import java.util.Arrays;
import java.util.List;
import java.util.Vector;

import org.junit.Test;

import io.qt.autotests.generated.SettingsTest;
import io.qt.core.QByteArray;
import io.qt.core.QSettings;
import io.qt.core.QVariant;
import io.qt.core.Qt;

public class TestQSettings extends QApplicationTest implements Serializable {
    private static final long serialVersionUID = 1L;

    @Test
    public void writeReadSettingsSimple() {
        {
			QSettings settings = new QSettings("QtJambi", "Test");
			settings.sync();

			settings.setValue("int", 5);
			settings.setValue("double", 5.000001d);
			settings.setValue("String", "String");

			settings.sync();
			settings.dispose();
		}
		{
			QSettings settings = new QSettings("QtJambi", "Test");
			settings.sync();

			assertEquals(5, QVariant.toInt(settings.value("int")));
			assertEquals(5.000001d, QVariant.toDouble(settings.value("double").toString()), 0.0);
			assertEquals("String", QVariant.toString(settings.value("String")));
			settings.dispose();
		}
    }

    @Test
    public void writeReadSettingsCollection() {
		{
			QSettings settings = new QSettings("QtJambi", "Test");
			settings.sync();

			List<String> list = new Vector<String>();
			for (int i = 0; i < 10; i++) {
				list.add("entry-" + i);
			}
			settings.setValue("test", list);
			settings.sync();
			settings.dispose();
		}
		{
			QSettings settings = new QSettings("QtJambi", "Test");
			settings.sync();

			List<?> list = (List<?>) settings.value("test", new Vector<String>());

			for (int i = 0; i < 10; i++) {
				assertEquals("entry-" + i, list.get(i));
			}
			settings.dispose();
		}
    }

    @Test
    public void readSettingsEmpty() {
        QSettings settings = new QSettings("QtJambi", "Test");
        settings.sync();

        String res = (String) settings.value("empty", "ok");
        assertEquals("ok", res);

        res = (String) settings.value("empty");
        assertNull(res);
		settings.dispose();
    }

    public class Custom implements Serializable {
        private static final long serialVersionUID = 1L;

        String name;
        int integer;
        Custom object;
    }

    @Test
    public void writeReadSettingsCustomClass() {
        {
			QSettings settings = new QSettings("QtJambi", "Test");
			settings.sync();

			Custom custom = new Custom();
			custom.name = "abc";
			custom.integer = 123;
			custom.object = new Custom();

			settings.setValue("custom", custom);
			settings.sync();
			settings.dispose();
		}
		{
			QSettings settings = new QSettings("QtJambi", "Test");
			settings.sync();

			Custom custom = (Custom) settings.value("custom");
			assertNotNull(custom);
			assertEquals(custom.name, "abc");
			assertEquals(custom.integer, 123);
			assertEquals(custom.object.getClass(), Custom.class);
			settings.dispose();
		}
    }
	
	@Test
    public void testCppToJava() throws IOException{
		File file = File.createTempFile("QtJambiTest_testCppToJava", ".ini");
		QSettings settings = new QSettings(file.getAbsolutePath(), QSettings.Format.IniFormat);
		SettingsTest.writeInt(settings);
		assertEquals(-5, settings.value("cpp-int"));
		SettingsTest.writeUInt(settings);
		assertEquals(39, settings.value("cpp-uint"));
		SettingsTest.writeInt64(settings);
		assertEquals(898776658768L, settings.value("cpp-int64"));
		SettingsTest.writeBool(settings);
		assertEquals(Boolean.TRUE, settings.value("cpp-bool"));
		SettingsTest.writeChar(settings);
		assertEquals((byte)578, settings.value("cpp-char"));
		SettingsTest.writeString(settings);
		assertEquals("\"test1, test2\"", settings.value("cpp-string"));
		SettingsTest.writeStringList(settings);
		assertEquals(Arrays.asList("test1", "test2"), settings.value("cpp-stringlist"));
		SettingsTest.writeIntList(settings);
		assertEquals(Arrays.asList(1, 2, 3, 4, 5), settings.value("cpp-intlist"));
		SettingsTest.writeByteArray(settings);
		assertEquals(new QByteArray("test"), settings.value("cpp-bytearray"));
		SettingsTest.writeEnum(settings);
		assertEquals(Qt.WindowType.WindowFullscreenButtonHint, settings.value("cpp-enum"));
		SettingsTest.writeFlag(settings);
		assertEquals(new Qt.WindowFlags(Qt.WindowType.WindowFullscreenButtonHint, Qt.WindowType.WindowTransparentForInput), settings.value("cpp-flag"));
//		settings.clear();
		settings.dispose();
		file.delete();
	}
	
	@Test
    public void testJavaToCpp() throws IOException{
		File file = File.createTempFile("QtJambiTest_testJavaToCpp", ".ini");
		QSettings settings = new QSettings(file.getAbsolutePath(), QSettings.Format.IniFormat);
		settings.setValue("java-enum", Qt.WindowType.WindowFullscreenButtonHint);
		assertTrue(SettingsTest.readEnum(settings));
		settings.setValue("java-flag", new Qt.WindowFlags(Qt.WindowType.WindowFullscreenButtonHint, Qt.WindowType.WindowTransparentForInput));
		assertTrue(SettingsTest.readFlag(settings));
		settings.setValue("java-int", (int)-5);
		assertTrue(SettingsTest.readInt(settings));
		settings.setValue("java-long", (long)898776658768L);
		assertTrue(SettingsTest.readLong(settings));
		settings.setValue("java-boolean", true);
		assertTrue(SettingsTest.readBoolean(settings));
		settings.setValue("java-byte", (byte)578);
		assertTrue(SettingsTest.readByte(settings));
		settings.setValue("java-string", "\"test1, test2\"");
		assertTrue(SettingsTest.readString(settings));
		settings.setValue("java-bytearray", new QByteArray("test"));
		assertTrue(SettingsTest.readByteArray(settings));
		settings.setValue("java-intlist", Arrays.asList(1, 2, 3, 4, 5));
		assertTrue(SettingsTest.readIntList(settings));
		settings.setValue("java-stringlist", Arrays.asList("test1", "test2"));
		assertTrue(SettingsTest.readStringList(settings));
//		settings.clear();
		settings.dispose();
		file.delete();
	}
	
	@Test
    public void testSettingsReadWrite() throws IOException{
		File file = File.createTempFile("QtJambiTest_testSettingsReadWrite", ".ini");
		QSettings settings = new QSettings(file.getAbsolutePath(), QSettings.Format.IniFormat);
		SettingsTest.write(settings, "java-enum", Qt.WindowType.WindowFullscreenButtonHint);
		assertTrue(SettingsTest.readEnum(settings));
		SettingsTest.write(settings, "java-flag", new Qt.WindowFlags(Qt.WindowType.WindowFullscreenButtonHint, Qt.WindowType.WindowTransparentForInput));
		assertTrue(SettingsTest.readFlag(settings));
		SettingsTest.write(settings, "java-int", (int)-5);
		assertTrue(SettingsTest.readInt(settings));
		SettingsTest.write(settings, "java-long", (long)898776658768L);
		assertTrue(SettingsTest.readLong(settings));
		SettingsTest.write(settings, "java-boolean", true);
		assertTrue(SettingsTest.readBoolean(settings));
		SettingsTest.write(settings, "java-byte", (byte)578);
		assertTrue(SettingsTest.readByte(settings));
		SettingsTest.write(settings, "java-string", "\"test1, test2\"");
		assertTrue(SettingsTest.readString(settings));
		SettingsTest.write(settings, "java-bytearray", new QByteArray("test"));
		assertTrue(SettingsTest.readByteArray(settings));
		SettingsTest.write(settings, "java-intlist", Arrays.asList(1, 2, 3, 4, 5));
		assertTrue(SettingsTest.readIntList(settings));
		SettingsTest.write(settings, "java-stringlist", Arrays.asList("test1", "test2"));
		assertTrue(SettingsTest.readStringList(settings));

		SettingsTest.writeEnum(settings);
		assertEquals(Qt.WindowType.WindowFullscreenButtonHint, SettingsTest.read(settings, "cpp-enum"));
		SettingsTest.writeFlag(settings);
		assertEquals(new Qt.WindowFlags(Qt.WindowType.WindowFullscreenButtonHint, Qt.WindowType.WindowTransparentForInput), SettingsTest.read(settings, "cpp-flag"));
		SettingsTest.writeInt(settings);
		assertEquals(-5, SettingsTest.read(settings, "cpp-int"));
		SettingsTest.writeUInt(settings);
		assertEquals(39, SettingsTest.read(settings, "cpp-uint"));
		SettingsTest.writeInt64(settings);
		assertEquals(898776658768L, SettingsTest.read(settings, "cpp-int64"));
		SettingsTest.writeBool(settings);
		assertEquals(Boolean.TRUE, SettingsTest.read(settings, "cpp-bool"));
		SettingsTest.writeChar(settings);
		assertEquals((byte)578, SettingsTest.read(settings, "cpp-char"));
		SettingsTest.writeString(settings);
		assertEquals("\"test1, test2\"", SettingsTest.read(settings, "cpp-string"));
		SettingsTest.writeByteArray(settings);
		assertEquals(new QByteArray("test"), SettingsTest.read(settings, "cpp-bytearray"));
		SettingsTest.writeIntList(settings);
		assertEquals(Arrays.asList(1, 2, 3, 4, 5), SettingsTest.read(settings, "cpp-intlist"));
		SettingsTest.writeStringList(settings);
		assertEquals(Arrays.asList("test1", "test2"), SettingsTest.read(settings, "cpp-stringlist"));

//		settings.clear();
		settings.dispose();
		file.delete();		
	}	

    public static void main(String args[]) {
        org.junit.runner.JUnitCore.main(TestQSettings.class.getName());
    }
}
