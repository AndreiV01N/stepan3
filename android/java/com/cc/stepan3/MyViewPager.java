package com.cc.stepan3;

import android.content.Context;
import android.support.v4.view.ViewPager;
import android.util.AttributeSet;
import android.view.MotionEvent;

public class MyViewPager extends ViewPager {
    private static boolean pagerEnabled;

    public MyViewPager(Context context, AttributeSet attrs) {
        super(context, attrs);
        pagerEnabled = true;
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent event) {
        return pagerEnabled && super.onInterceptTouchEvent(event);
    }

    public static void setPagingEnabled(boolean e) {
        pagerEnabled = e;
    }
}
