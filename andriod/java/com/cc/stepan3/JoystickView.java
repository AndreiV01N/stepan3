/**
 * Copyright 2011 Thomas Niederberger
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modifications and improvements done by Kristian Lauszus, TKJ Electronics.
 */

package com.cc.stepan3;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

public class JoystickView extends View {
    private OnJoystickChangeListener listener;

    private final int holo_blue_dark = 0xFF0099CC;
    private final int buttonGray = 0xFF5C5C5C;

    private int buttonColor = buttonGray;
    private int actionCounter = 0;

    private float x, y;
    private double lastX, lastY;
    private float buttonRadius;
    private float joystickRadius = 0;
    private float centerX;
    private float centerY;

    private Paint p = new Paint();

    public JoystickView(Context context) {
        super(context);
    }

    public JoystickView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public JoystickView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, widthMeasureSpec); // Make the layout square
        setMeasuredDimension(getMeasuredWidth(), getMeasuredWidth());
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (joystickRadius == 0) {
            joystickRadius = (float) getWidth() / 3;
            buttonRadius = joystickRadius / 2;
            centerX = (float) getWidth() - joystickRadius - buttonRadius;
            centerY = (float) getHeight() - joystickRadius - buttonRadius;
            x = centerX;
            y = centerY;
        }

        p.setStyle(Paint.Style.STROKE);
        p.setStrokeWidth(3);
        p.setColor(buttonGray);
        canvas.drawCircle(centerX, centerY, joystickRadius, p);
        canvas.drawCircle(centerX, centerY, joystickRadius / 2, p);
        p.setColor(buttonColor);
        p.setStyle(Paint.Style.FILL);
        canvas.drawCircle(x, y, buttonRadius, p);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int actionFilter = 1; // only every 'actionFilter' action is processed

        x = event.getX();
        y = event.getY();
        float abs = (float) Math.sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));

        if (abs > joystickRadius) {
            x = ((x - centerX) * joystickRadius / abs + centerX);
            y = ((y - centerY) * joystickRadius / abs + centerY);
        }

        if (lastX == 0 && lastY == 0 && (getXValue() > 0.50 || getXValue() < -0.50 || getYValue() > 0.50 || getYValue() < -0.50)) {
            x = centerX;
            y = centerY;
            return true;
        }

        lastX = getXValue();
        lastY = getYValue();

        invalidate();

        if (listener != null) {
            int actionMask = event.getActionMasked();
            if (actionMask == MotionEvent.ACTION_DOWN) {
                buttonColor = holo_blue_dark;
                listener.setOnTouchListener(getXValue(), getYValue());
                return true;
            } else if (actionMask == MotionEvent.ACTION_MOVE && actionCounter % actionFilter == 0) {
                buttonColor = holo_blue_dark;
                listener.setOnMovedListener(getXValue(), getYValue());
                actionCounter++;
                return true;
            } else if (actionMask == MotionEvent.ACTION_UP || actionMask == MotionEvent.ACTION_CANCEL) {
                buttonColor = buttonGray;
                x = centerX;
                y = centerY;
                lastX = 0;
                lastY = 0;
                listener.setOnReleaseListener(lastX, lastY);
                return true;
            }
        }
        actionCounter++;
        return false;
    }

    public double getXValue() {
        return (x - centerX) / joystickRadius; // X-axis is positive at the right side
    }

    public double getYValue() {
        return -((y - centerY) / joystickRadius); // Y-axis should be positive upwards
    }

    public void setOnJoystickChangeListener(OnJoystickChangeListener listener) {
        this.listener = listener;
    }

    public interface OnJoystickChangeListener {
        public void setOnTouchListener(double xValue, double yValue);

        public void setOnMovedListener(double xValue, double yValue);

        public void setOnReleaseListener(double xValue, double yValue);
    }
}
