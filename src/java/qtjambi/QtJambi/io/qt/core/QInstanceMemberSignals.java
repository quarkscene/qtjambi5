/****************************************************************************
**
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
package io.qt.core;

import java.lang.ref.WeakReference;
import java.util.Objects;
import java.util.function.Supplier;

import io.qt.QNoDefaultValueException;
import io.qt.QtSignalEmitterInterface;
import io.qt.QtUninvokable;
import io.qt.core.QMetaObject.AbstractPrivateSignal0;
import io.qt.core.QMetaObject.AbstractPrivateSignal1;
import io.qt.core.QMetaObject.AbstractPrivateSignal2;
import io.qt.core.QMetaObject.AbstractPrivateSignal3;
import io.qt.core.QMetaObject.AbstractPrivateSignal4;
import io.qt.core.QMetaObject.AbstractPrivateSignal5;
import io.qt.core.QMetaObject.AbstractPrivateSignal6;
import io.qt.core.QMetaObject.AbstractPrivateSignal7;
import io.qt.core.QMetaObject.AbstractPrivateSignal8;
import io.qt.core.QMetaObject.AbstractPrivateSignal9;
import io.qt.core.QMetaObject.AbstractPublicSignal0;
import io.qt.core.QMetaObject.AbstractPublicSignal1;
import io.qt.core.QMetaObject.AbstractPublicSignal2;
import io.qt.core.QMetaObject.AbstractPublicSignal3;
import io.qt.core.QMetaObject.AbstractPublicSignal4;
import io.qt.core.QMetaObject.AbstractPublicSignal5;
import io.qt.core.QMetaObject.AbstractPublicSignal6;
import io.qt.core.QMetaObject.AbstractPublicSignal7;
import io.qt.core.QMetaObject.AbstractPublicSignal8;
import io.qt.core.QMetaObject.AbstractPublicSignal9;
import io.qt.core.QMetaObject.AbstractSignal2Default1;
import io.qt.core.QMetaObject.AbstractSignal3Default1;
import io.qt.core.QMetaObject.AbstractSignal3Default2;
import io.qt.core.QMetaObject.AbstractSignal4Default1;
import io.qt.core.QMetaObject.AbstractSignal4Default2;
import io.qt.core.QMetaObject.AbstractSignal4Default3;
import io.qt.core.QMetaObject.AbstractSignal5Default1;
import io.qt.core.QMetaObject.AbstractSignal5Default2;
import io.qt.core.QMetaObject.AbstractSignal5Default3;
import io.qt.core.QMetaObject.AbstractSignal5Default4;
import io.qt.core.QMetaObject.AbstractSignal6Default1;
import io.qt.core.QMetaObject.AbstractSignal6Default2;
import io.qt.core.QMetaObject.AbstractSignal6Default3;
import io.qt.core.QMetaObject.AbstractSignal6Default4;
import io.qt.core.QMetaObject.AbstractSignal6Default5;
import io.qt.core.QMetaObject.AbstractSignal7Default1;
import io.qt.core.QMetaObject.AbstractSignal7Default2;
import io.qt.core.QMetaObject.AbstractSignal7Default3;
import io.qt.core.QMetaObject.AbstractSignal7Default4;
import io.qt.core.QMetaObject.AbstractSignal7Default5;
import io.qt.core.QMetaObject.AbstractSignal7Default6;
import io.qt.core.QMetaObject.AbstractSignal8Default1;
import io.qt.core.QMetaObject.AbstractSignal8Default2;
import io.qt.core.QMetaObject.AbstractSignal8Default3;
import io.qt.core.QMetaObject.AbstractSignal8Default4;
import io.qt.core.QMetaObject.AbstractSignal8Default5;
import io.qt.core.QMetaObject.AbstractSignal8Default6;
import io.qt.core.QMetaObject.AbstractSignal8Default7;
import io.qt.core.QMetaObject.AbstractSignal9Default1;
import io.qt.core.QMetaObject.AbstractSignal9Default2;
import io.qt.core.QMetaObject.AbstractSignal9Default3;
import io.qt.core.QMetaObject.AbstractSignal9Default4;
import io.qt.core.QMetaObject.AbstractSignal9Default5;
import io.qt.core.QMetaObject.AbstractSignal9Default6;
import io.qt.core.QMetaObject.AbstractSignal9Default7;
import io.qt.core.QMetaObject.AbstractSignal9Default8;
import io.qt.internal.QtJambiInternal;

public interface QInstanceMemberSignals {
    /**
     * Use this signal class to define a static signal.
     */
    public static final class Signal0 extends AbstractPublicSignal0 {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal0(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     */
    public static final class Signal1<A> extends AbstractPublicSignal1<A> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal1(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     */
    public static final class Signal2<A,B> extends AbstractPublicSignal2<A,B> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal2(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     */
    public static final class Signal3<A,B,C> extends AbstractPublicSignal3<A,B,C> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal3(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     */
    public static final class Signal4<A,B,C,D> extends AbstractPublicSignal4<A,B,C,D> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal4(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     * @param <E> The type of the single parameter of the signal.
     */
    public static final class Signal5<A,B,C,D,E> extends AbstractPublicSignal5<A,B,C,D,E> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal5(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     * @param <E> The type of the single parameter of the signal.
     * @param <F> The type of the single parameter of the signal.
     */
    public static final class Signal6<A,B,C,D,E,F> extends AbstractPublicSignal6<A,B,C,D,E,F> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal6(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     * @param <E> The type of the single parameter of the signal.
     * @param <F> The type of the single parameter of the signal.
     * @param <G> The type of the single parameter of the signal.
     */
    public static final class Signal7<A,B,C,D,E,F,G> extends AbstractPublicSignal7<A,B,C,D,E,F,G> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal7(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     * @param <E> The type of the single parameter of the signal.
     * @param <F> The type of the single parameter of the signal.
     * @param <G> The type of the single parameter of the signal.
     * @param <H> The type of the single parameter of the signal.
     */
    public static final class Signal8<A,B,C,D,E,F,G,H> extends AbstractPublicSignal8<A,B,C,D,E,F,G,H> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal8(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     * @param <E> The type of the single parameter of the signal.
     * @param <F> The type of the single parameter of the signal.
     * @param <G> The type of the single parameter of the signal.
     * @param <H> The type of the single parameter of the signal.
     * @param <I> The type of the single parameter of the signal.
     */
    public static final class Signal9<A,B,C,D,E,F,G,H,I> extends AbstractPublicSignal9<A,B,C,D,E,F,G,H,I> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal9(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }

    /**
     * Use this signal class to define a static signal.
     */
    public static final class PrivateSignal0 extends AbstractPrivateSignal0 {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> PrivateSignal0(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@QtUninvokable
        private void emit() {
            emitSignal();
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     */
    public static final class PrivateSignal1<A> extends AbstractPrivateSignal1<A> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> PrivateSignal1(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@QtUninvokable
        private void emit(A arg1) {
            emitSignal(arg1);
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     */
    public static final class PrivateSignal2<A,B> extends AbstractPrivateSignal2<A,B> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> PrivateSignal2(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@QtUninvokable
        private void emit(A arg1, B arg2) {
            emitSignal(arg1, arg2);
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     */
    public static final class PrivateSignal3<A,B,C> extends AbstractPrivateSignal3<A,B,C> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> PrivateSignal3(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@QtUninvokable
        private void emit(A arg1, B arg2, C arg3) {
            emitSignal(arg1, arg2, arg3);
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     */
    public static final class PrivateSignal4<A,B,C,D> extends AbstractPrivateSignal4<A,B,C,D> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> PrivateSignal4(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@QtUninvokable
        private void emit(A arg1, B arg2, C arg3, D arg4) {
            emitSignal(arg1, arg2, arg3, arg4);
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     * @param <E> The type of the single parameter of the signal.
     */
    public static final class PrivateSignal5<A,B,C,D,E> extends AbstractPrivateSignal5<A,B,C,D,E> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> PrivateSignal5(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@QtUninvokable
        private void emit(A arg1, B arg2, C arg3, D arg4, E arg5) {
            emitSignal(arg1, arg2, arg3, arg4, arg5);
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     * @param <E> The type of the single parameter of the signal.
     * @param <F> The type of the single parameter of the signal.
     */
    public static final class PrivateSignal6<A,B,C,D,E,F> extends AbstractPrivateSignal6<A,B,C,D,E,F> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> PrivateSignal6(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@QtUninvokable
        private void emit(A arg1, B arg2, C arg3, D arg4, E arg5, F arg6) {
            emitSignal(arg1, arg2, arg3, arg4, arg5, arg6);
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     * @param <E> The type of the single parameter of the signal.
     * @param <F> The type of the single parameter of the signal.
     * @param <G> The type of the single parameter of the signal.
     */
    public static final class PrivateSignal7<A,B,C,D,E,F,G> extends AbstractPrivateSignal7<A,B,C,D,E,F,G> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> PrivateSignal7(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@QtUninvokable
        private void emit(A arg1, B arg2, C arg3, D arg4, E arg5, F arg6, G arg7) {
            emitSignal(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     * @param <E> The type of the single parameter of the signal.
     * @param <F> The type of the single parameter of the signal.
     * @param <G> The type of the single parameter of the signal.
     * @param <H> The type of the single parameter of the signal.
     */
    public static final class PrivateSignal8<A,B,C,D,E,F,G,H> extends AbstractPrivateSignal8<A,B,C,D,E,F,G,H> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> PrivateSignal8(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@QtUninvokable
        private void emit(A arg1, B arg2, C arg3, D arg4, E arg5, F arg6, G arg7, H arg8) {
            emitSignal(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    /**
     * Use this signal class to define a static signal.
     *
     * @param <A> The type of the single parameter of the signal.
     * @param <B> The type of the single parameter of the signal.
     * @param <C> The type of the single parameter of the signal.
     * @param <D> The type of the single parameter of the signal.
     * @param <E> The type of the single parameter of the signal.
     * @param <F> The type of the single parameter of the signal.
     * @param <G> The type of the single parameter of the signal.
     * @param <H> The type of the single parameter of the signal.
     * @param <I> The type of the single parameter of the signal.
     */
    public static final class PrivateSignal9<A,B,C,D,E,F,G,H,I> extends AbstractPrivateSignal9<A,B,C,D,E,F,G,H,I> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> PrivateSignal9(Q containingObject) {
			super();
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@QtUninvokable
        private void emit(A arg1, B arg2, C arg3, D arg4, E arg5, F arg6, G arg7, H arg8, I arg9) {
            emitSignal(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal1Default1<A> extends AbstractPublicSignal1<A> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal1Default1(Q containingObject, Supplier<A> arg1Default){
    		super();
    		if(arg1Default!=null){
    			this.arg1Default = arg1Default;
    		}else{
        		throw new QNoDefaultValueException(1);
        	}
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	private final Supplier<A> arg1Default;
        
        public void emit() {
       		emit(arg1Default.get());
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal2Default1<A, B> extends AbstractSignal2Default1<A, B>{
		public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal2Default1(Q containingObject, Supplier<B> arg2Default) {
			super(arg2Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
	}
    
    public static final class Signal2Default2<A, B> extends AbstractSignal2Default1<A, B> {
    	
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal2Default2(Q containingObject, Supplier<A> arg1Default, Supplier<B> arg2Default){
    		super(arg2Default);
    		if(arg1Default!=null){
    			this.arg1Default = arg1Default;
    		}else{
        		throw new QNoDefaultValueException(1);
        	}
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	private final Supplier<A> arg1Default;
        
        public void emit() {
       		emit(arg1Default.get());
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal3Default1<A, B, C> extends AbstractSignal3Default1<A, B, C> {
		public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal3Default1(Q containingObject, Supplier<C> arg3Default) {
			super(arg3Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal3Default2<A, B, C> extends AbstractSignal3Default2<A, B, C> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal3Default2(Q containingObject, Supplier<B> arg2Default, Supplier<C> arg3Default){
    		super(arg2Default, arg3Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal3Default3<A, B, C> extends AbstractSignal3Default2<A, B, C> {
    	
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal3Default3(Q containingObject, Supplier<A> arg1Default, Supplier<B> arg2Default, Supplier<C> arg3Default){
    		super(arg2Default, arg3Default);
    		if(arg2Default!=null){
    			this.arg1Default = arg1Default;
    		}else{
        		throw new QNoDefaultValueException(1);
        	}
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	private final Supplier<A> arg1Default;
        
        public void emit() {
       		emit(arg1Default.get());
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal4Default1<A, B, C, D> extends AbstractSignal4Default1<A, B, C, D>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal4Default1(Q containingObject, Supplier<D> arg4Default){
    		super(arg4Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal4Default2<A, B, C, D> extends AbstractSignal4Default2<A, B, C, D>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal4Default2(Q containingObject, Supplier<C> arg3Default, Supplier<D> arg4Default){
    		super(arg3Default, arg4Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal4Default3<A, B, C, D> extends AbstractSignal4Default3<A, B, C, D>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal4Default3(Q containingObject, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default){
    		super(arg2Default, arg3Default, arg4Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal4Default4<A, B, C, D> extends AbstractSignal4Default3<A, B, C, D> {
    	
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal4Default4(
    			Q containingObject, 
    			Supplier<A> arg1Default, Supplier<B> arg2Default, 
    			Supplier<C> arg3Default, Supplier<D> arg4Default){
    		super(arg2Default, arg3Default, arg4Default);
    		if(arg1Default!=null){
    			this.arg1Default = arg1Default;
    		}else{
        		throw new QNoDefaultValueException(1);
        	}
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	private final Supplier<A> arg1Default;
        
        public void emit() {
       		emit(arg1Default.get());
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal5Default1<A, B, C, D, E> extends AbstractSignal5Default1<A, B, C, D, E>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal5Default1(Q containingObject, Supplier<E> arg5Default){
    		super(arg5Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal5Default2<A, B, C, D, E> extends AbstractSignal5Default2<A, B, C, D, E>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal5Default2(Q containingObject, Supplier<D> arg4Default, Supplier<E> arg5Default){
    		super(arg4Default, arg5Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal5Default3<A, B, C, D, E> extends AbstractSignal5Default3<A, B, C, D, E>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal5Default3(Q containingObject, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default){
    		super(arg3Default, arg4Default, arg5Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal5Default4<A, B, C, D, E> extends AbstractSignal5Default4<A, B, C, D, E>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal5Default4(Q containingObject, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default){
    		super(arg2Default, arg3Default, arg4Default, arg5Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal5Default5<A, B, C, D, E> extends AbstractSignal5Default4<A, B, C, D, E>{
    	
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal5Default5(Q containingObject, Supplier<A> arg1Default, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default){
    		super(arg2Default, arg3Default, arg4Default, arg5Default);
    		if(arg1Default!=null){
    			this.arg1Default = arg1Default;
    		}else{
        		throw new QNoDefaultValueException(1);
        	}
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	private final Supplier<A> arg1Default;
        
        public void emit() {
       		emit(arg1Default.get());
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal6Default1<A, B, C, D, E, F> extends AbstractSignal6Default1<A, B, C, D, E, F>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal6Default1(Q containingObject, Supplier<F> arg6Default){
    		super(arg6Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal6Default2<A, B, C, D, E, F> extends AbstractSignal6Default2<A, B, C, D, E, F>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal6Default2(Q containingObject, Supplier<E> arg5Default, Supplier<F> arg6Default){
    		super(arg5Default, arg6Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal6Default3<A, B, C, D, E, F> extends AbstractSignal6Default3<A, B, C, D, E, F>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal6Default3(Q containingObject, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default){
    		super(arg4Default, arg5Default, arg6Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal6Default4<A, B, C, D, E, F> extends AbstractSignal6Default4<A, B, C, D, E, F>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal6Default4(Q containingObject, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default){
    		super(arg3Default, arg4Default, arg5Default, arg6Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal6Default5<A, B, C, D, E, F> extends AbstractSignal6Default5<A, B, C, D, E, F>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal6Default5(Q containingObject, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default){
    		super(arg2Default, arg3Default, arg4Default, arg5Default, arg6Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal6Default6<A, B, C, D, E, F> extends AbstractSignal6Default5<A, B, C, D, E, F>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal6Default6(Q containingObject, Supplier<A> arg1Default, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default){
    		super(arg2Default, arg3Default, arg4Default, arg5Default, arg6Default);
			this.arg1Default = arg1Default;
    		if(this.arg1Default==null){
        		throw new QNoDefaultValueException(1);
        	}
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	private final Supplier<A> arg1Default;
        
    	public void emit() {
       		emit(arg1Default.get());
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal7Default1<A, B, C, D, E, F, G> extends AbstractSignal7Default1<A, B, C, D, E, F, G>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal7Default1(Q containingObject, Supplier<G> arg7Default){
    		super(arg7Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal7Default2<A, B, C, D, E, F, G> extends AbstractSignal7Default2<A, B, C, D, E, F, G> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal7Default2(Q containingObject, Supplier<F> arg6Default, Supplier<G> arg7Default){
    		super(arg6Default, arg7Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal7Default3<A, B, C, D, E, F, G> extends AbstractSignal7Default3<A, B, C, D, E, F, G> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal7Default3(Q containingObject, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default){
    		super(arg5Default, arg6Default, arg7Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal7Default4<A, B, C, D, E, F, G> extends AbstractSignal7Default4<A, B, C, D, E, F, G> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal7Default4(Q containingObject, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default){
    		super(arg4Default, arg5Default, arg6Default, arg7Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal7Default5<A, B, C, D, E, F, G> extends AbstractSignal7Default5<A, B, C, D, E, F, G> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal7Default5(Q containingObject, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default){
    		super(arg3Default, arg4Default, arg5Default, arg6Default, arg7Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal7Default6<A, B, C, D, E, F, G> extends AbstractSignal7Default6<A, B, C, D, E, F, G> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal7Default6(Q containingObject, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default){
    		super(arg2Default, arg3Default, arg4Default, arg5Default, arg6Default, arg7Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal7Default7<A, B, C, D, E, F, G> extends AbstractSignal7Default6<A, B, C, D, E, F, G> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal7Default7(Q containingObject, Supplier<A> arg1Default, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default){
    		super(arg2Default, arg3Default, arg4Default, arg5Default, arg6Default, arg7Default);
			this.arg1Default = arg1Default;
    		if(this.arg1Default==null){
        		throw new QNoDefaultValueException(2);
        	}
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	private final Supplier<A> arg1Default;
        
        public void emit() {
       		emit(arg1Default.get());
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal8Default1<A, B, C, D, E, F, G, H> extends AbstractSignal8Default1<A, B, C, D, E, F, G, H>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal8Default1(Q containingObject, Supplier<H> arg8Default){
    		super(arg8Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal8Default2<A, B, C, D, E, F, G, H> extends AbstractSignal8Default2<A, B, C, D, E, F, G, H> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal8Default2(Q containingObject, Supplier<G> arg7Default, Supplier<H> arg8Default){
    		super(arg7Default, arg8Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal8Default3<A, B, C, D, E, F, G, H> extends AbstractSignal8Default3<A, B, C, D, E, F, G, H> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal8Default3(Q containingObject, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default){
    		super(arg6Default, arg7Default, arg8Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal8Default4<A, B, C, D, E, F, G, H> extends AbstractSignal8Default4<A, B, C, D, E, F, G, H> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal8Default4(Q containingObject, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default){
    		super(arg5Default, arg6Default, arg7Default, arg8Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal8Default5<A, B, C, D, E, F, G, H> extends AbstractSignal8Default5<A, B, C, D, E, F, G, H> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal8Default5(Q containingObject, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default){
    		super(arg4Default, arg5Default, arg6Default, arg7Default, arg8Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal8Default6<A, B, C, D, E, F, G, H> extends AbstractSignal8Default6<A, B, C, D, E, F, G, H> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal8Default6(Q containingObject, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default){
    		super(arg3Default, arg4Default, arg5Default, arg6Default, arg7Default, arg8Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal8Default7<A, B, C, D, E, F, G, H> extends AbstractSignal8Default7<A, B, C, D, E, F, G, H> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal8Default7(Q containingObject, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default){
    		super(arg2Default, arg3Default, arg4Default, arg5Default, arg6Default, arg7Default, arg8Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal8Default8<A, B, C, D, E, F, G, H> extends AbstractSignal8Default7<A, B, C, D, E, F, G, H> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal8Default8(Q containingObject, Supplier<A> arg1Default, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default){
    		super(arg2Default, arg3Default, arg4Default, arg5Default, arg6Default, arg7Default, arg8Default);
			this.arg1Default = arg1Default;
			if(this.arg1Default==null){
        		throw new QNoDefaultValueException(1);
        	}
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	private final Supplier<A> arg1Default;
        
        public void emit() {
       		emit(arg1Default.get());
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal9Default1<A, B, C, D, E, F, G, H, I> extends AbstractSignal9Default1<A, B, C, D, E, F, G, H, I>{
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal9Default1(Q containingObject, Supplier<I> arg9Default){
    		super(arg9Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal9Default2<A, B, C, D, E, F, G, H, I> extends AbstractSignal9Default2<A, B, C, D, E, F, G, H, I> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal9Default2(Q containingObject, Supplier<H> arg8Default, Supplier<I> arg9Default){
    		super(arg8Default, arg9Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal9Default3<A, B, C, D, E, F, G, H, I> extends AbstractSignal9Default3<A, B, C, D, E, F, G, H, I> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal9Default3(Q containingObject, Supplier<G> arg7Default, Supplier<H> arg8Default, Supplier<I> arg9Default){
    		super(arg7Default, arg8Default, arg9Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal9Default4<A, B, C, D, E, F, G, H, I> extends AbstractSignal9Default4<A, B, C, D, E, F, G, H, I> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal9Default4(Q containingObject, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default, Supplier<I> arg9Default){
    		super(arg6Default, arg7Default, arg8Default, arg9Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal9Default5<A, B, C, D, E, F, G, H, I> extends AbstractSignal9Default5<A, B, C, D, E, F, G, H, I> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal9Default5(Q containingObject, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default, Supplier<I> arg9Default){
    		super(arg5Default, arg6Default, arg7Default, arg8Default, arg9Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal9Default6<A, B, C, D, E, F, G, H, I> extends AbstractSignal9Default6<A, B, C, D, E, F, G, H, I> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal9Default6(Q containingObject, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default, Supplier<I> arg9Default){
    		super(arg4Default, arg5Default, arg6Default, arg7Default, arg8Default, arg9Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
		}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal9Default7<A, B, C, D, E, F, G, H, I> extends AbstractSignal9Default7<A, B, C, D, E, F, G, H, I> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal9Default7(Q containingObject, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default, Supplier<I> arg9Default){
    		super(arg3Default, arg4Default, arg5Default, arg6Default, arg7Default, arg8Default, arg9Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal9Default8<A, B, C, D, E, F, G, H, I> extends AbstractSignal9Default8<A, B, C, D, E, F, G, H, I> {
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal9Default8(Q containingObject, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default, Supplier<I> arg9Default){
    		super(arg2Default, arg3Default, arg4Default, arg5Default, arg6Default, arg7Default, arg8Default, arg9Default);
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static final class Signal9Default9<A, B, C, D, E, F, G, H, I> extends AbstractSignal9Default8<A, B, C, D, E, F, G, H, I> {
    	
    	public <Q extends Object & QtSignalEmitterInterface & QInstanceMemberSignals> Signal9Default9(Q containingObject, Supplier<A> arg1Default, Supplier<B> arg2Default, Supplier<C> arg3Default, Supplier<D> arg4Default, Supplier<E> arg5Default, Supplier<F> arg6Default, Supplier<G> arg7Default, Supplier<H> arg8Default, Supplier<I> arg9Default){
    		super(arg2Default, arg3Default, arg4Default, arg5Default, arg6Default, arg7Default, arg8Default, arg9Default);
			this.arg1Default = arg1Default;
    		if(this.arg1Default==null){
        		throw new QNoDefaultValueException(1);
        	}
			this.containingObject = new WeakReference<>(Objects.requireNonNull(containingObject));
    	}
    	
    	private final Supplier<A> arg1Default;
        
        public void emit() {
       		emit(arg1Default.get());
        }
    	
    	@Override
		public QtSignalEmitterInterface containingObject() {
			return containingObject.get();
		}
    	
    	private final WeakReference<QtSignalEmitterInterface> containingObject;    	
    }
    
    public static void emit(PrivateSignal0 signal) throws io.qt.QSignalAccessException {
    	Class<?> callerClass = QStaticMemberSignals.stackWalker.getCallerClass();
    	Class<?> signalDeclaringClass = QtJambiInternal.signalDeclaringClass(signal);
    	if(callerClass==signalDeclaringClass) {
    		signal.emit();
    	}else {
    		throw new io.qt.QSignalAccessException(signalDeclaringClass, signal.name(), callerClass);
    	}
    }
    
    public static <A> void emit(PrivateSignal1<A> signal, A arg1) throws io.qt.QSignalAccessException {
    	Class<?> callerClass = QStaticMemberSignals.stackWalker.getCallerClass();
    	Class<?> signalDeclaringClass = QtJambiInternal.signalDeclaringClass(signal);
    	if(callerClass==signalDeclaringClass) {
    		signal.emit(arg1);
    	}else {
    		throw new io.qt.QSignalAccessException(signalDeclaringClass, signal.name(), callerClass);
    	}
    }
    
    public static <A,B> void emit(PrivateSignal2<A,B> signal, A arg1, B arg2) throws io.qt.QSignalAccessException {
    	Class<?> callerClass = QStaticMemberSignals.stackWalker.getCallerClass();
    	Class<?> signalDeclaringClass = QtJambiInternal.signalDeclaringClass(signal);
    	if(callerClass==signalDeclaringClass) {
    		signal.emit(arg1, arg2);
    	}else {
    		throw new io.qt.QSignalAccessException(signalDeclaringClass, signal.name(), callerClass);
    	}
    }
    
    public static <A,B,C> void emit(PrivateSignal3<A,B,C> signal, A arg1, B arg2, C arg3) throws io.qt.QSignalAccessException {
    	Class<?> callerClass = QStaticMemberSignals.stackWalker.getCallerClass();
    	Class<?> signalDeclaringClass = QtJambiInternal.signalDeclaringClass(signal);
    	if(callerClass==signalDeclaringClass) {
    		signal.emit(arg1, arg2, arg3);
    	}else {
    		throw new io.qt.QSignalAccessException(signalDeclaringClass, signal.name(), callerClass);
    	}
    }
    
    public static <A,B,C,D> void emit(PrivateSignal4<A,B,C,D> signal, A arg1, B arg2, C arg3, D arg4) throws io.qt.QSignalAccessException {
    	Class<?> callerClass = QStaticMemberSignals.stackWalker.getCallerClass();
    	Class<?> signalDeclaringClass = QtJambiInternal.signalDeclaringClass(signal);
    	if(callerClass==signalDeclaringClass) {
    		signal.emit(arg1, arg2, arg3, arg4);
    	}else {
    		throw new io.qt.QSignalAccessException(signalDeclaringClass, signal.name(), callerClass);
    	}
    }
    
    public static <A,B,C,D,E> void emit(PrivateSignal5<A,B,C,D,E> signal, A arg1, B arg2, C arg3, D arg4, E arg5) throws io.qt.QSignalAccessException {
    	Class<?> callerClass = QStaticMemberSignals.stackWalker.getCallerClass();
    	Class<?> signalDeclaringClass = QtJambiInternal.signalDeclaringClass(signal);
    	if(callerClass==signalDeclaringClass) {
    		signal.emit(arg1, arg2, arg3, arg4, arg5);
    	}else {
    		throw new io.qt.QSignalAccessException(signalDeclaringClass, signal.name(), callerClass);
    	}
    }
    
    public static <A,B,C,D,E,F> void emit(PrivateSignal6<A,B,C,D,E,F> signal, A arg1, B arg2, C arg3, D arg4, E arg5, F arg6) throws io.qt.QSignalAccessException {
    	Class<?> callerClass = QStaticMemberSignals.stackWalker.getCallerClass();
    	Class<?> signalDeclaringClass = QtJambiInternal.signalDeclaringClass(signal);
    	if(callerClass==signalDeclaringClass) {
    		signal.emit(arg1, arg2, arg3, arg4, arg5, arg6);
    	}else {
    		throw new io.qt.QSignalAccessException(signalDeclaringClass, signal.name(), callerClass);
    	}
    }
    
    public static <A,B,C,D,E,F,G> void emit(PrivateSignal7<A,B,C,D,E,F,G> signal, A arg1, B arg2, C arg3, D arg4, E arg5, F arg6, G arg7) throws io.qt.QSignalAccessException {
    	Class<?> callerClass = QStaticMemberSignals.stackWalker.getCallerClass();
    	Class<?> signalDeclaringClass = QtJambiInternal.signalDeclaringClass(signal);
    	if(callerClass==signalDeclaringClass) {
    		signal.emit(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    	}else {
    		throw new io.qt.QSignalAccessException(signalDeclaringClass, signal.name(), callerClass);
    	}
    }
    
    public static <A,B,C,D,E,F,G,H> void emit(PrivateSignal8<A,B,C,D,E,F,G,H> signal, A arg1, B arg2, C arg3, D arg4, E arg5, F arg6, G arg7, H arg8) throws io.qt.QSignalAccessException {
    	Class<?> callerClass = QStaticMemberSignals.stackWalker.getCallerClass();
    	Class<?> signalDeclaringClass = QtJambiInternal.signalDeclaringClass(signal);
    	if(callerClass==signalDeclaringClass) {
    		signal.emit(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    	}else {
    		throw new io.qt.QSignalAccessException(signalDeclaringClass, signal.name(), callerClass);
    	}
    }
    
    public static <A,B,C,D,E,F,G,H,I> void emit(PrivateSignal9<A,B,C,D,E,F,G,H,I> signal, A arg1, B arg2, C arg3, D arg4, E arg5, F arg6, G arg7, H arg8, I arg9) throws io.qt.QSignalAccessException {
    	Class<?> callerClass = QStaticMemberSignals.stackWalker.getCallerClass();
    	Class<?> signalDeclaringClass = QtJambiInternal.signalDeclaringClass(signal);
    	if(callerClass==signalDeclaringClass) {
    		signal.emit(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    	}else {
    		throw new io.qt.QSignalAccessException(signalDeclaringClass, signal.name(), callerClass);
    	}
    }
}
