package com.cc.stepan3;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.widget.Button;
import android.widget.TextView;

import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.Locale;


public class Fragment_1 extends Fragment implements JoystickView.OnJoystickChangeListener {
    private DecimalFormat d = (DecimalFormat) NumberFormat.getNumberInstance(Locale.ENGLISH);
    private TextView text;
    private double xValue, yValue;
    private boolean joystickReleased;
    private SharedPreferences mSharedPrefs;
    private String ipAddr;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSharedPrefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_1, container, false);

        WebView mWebView = rootView.findViewById(R.id.frgm_1_webview);

        JoystickView mJoystickView = rootView.findViewById(R.id.frgm_1_joystick);
        mJoystickView.setOnJoystickChangeListener(this);
        MainActivity.joystickReleased = true;

        text = rootView.findViewById(R.id.frgm_1_text);
        text.setText(R.string.defaultJoystickValue);

        String defaultVideoURL = getResources().getString(R.string.default_video_url);
        String videoURL = mSharedPrefs.getString(getString(R.string.preference_video_url), defaultVideoURL);

        String defaultIPaddr = getResources().getString(R.string.default_ip_addr);
        ipAddr = mSharedPrefs.getString(getString(R.string.preference_ip_addr), defaultIPaddr);

        mWebView.getSettings().setJavaScriptEnabled(true);
        mWebView.loadUrl(videoURL);

        Button btnWAKEUP = rootView.findViewById(R.id.frgm_1_button1);
        btnWAKEUP.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr,
                                        getString(R.string.udp_port),
                                        getString(R.string.wakeup_command, "1"));
            }
        });

        Button btnSLEEP = rootView.findViewById(R.id.frgm_1_button2);
        btnSLEEP.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr,
                        getString(R.string.udp_port),
                        getString(R.string.sleep_command, "1"));
            }
        });

        Button btnCTRL = rootView.findViewById(R.id.frgm_1_button3);
        btnCTRL.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr,
                        getString(R.string.udp_port),
                        getString(R.string.control_command, "1"));
            }
        });

        Button btnRESET = rootView.findViewById(R.id.frgm_1_button4);
        btnRESET.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr,
                        getString(R.string.udp_port),
                        getString(R.string.reset_command, "1"));
            }
        });

        Button btnAUTOROUTE = rootView.findViewById(R.id.frgm_1_button5);
        btnAUTOROUTE.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr,
                        getString(R.string.udp_port),
                        getString(R.string.autoroute_command, "1"));
            }
        });

        Button btnTEST = rootView.findViewById(R.id.frgm_1_button6);
        btnTEST.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr,
                        getString(R.string.udp_port),
                        getString(R.string.test_command, "1"));
            }
        });

        return rootView;
    }

    private void newData(double xValue, double yValue, boolean joystickReleased) {
        if (xValue == 0 && yValue == 0)
            joystickReleased = true;

        MyViewPager.setPagingEnabled(joystickReleased);
        MainActivity.joystickReleased = joystickReleased;
        this.joystickReleased = joystickReleased;

        text.setText(getString(R.string.x_y_label,
                               String.format(Locale.ENGLISH, "%.2f", xValue),
                               String.format(Locale.ENGLISH, "%.2f", yValue)));

        new UdpClient().execute(ipAddr,
                getString(R.string.udp_port),
                getString(R.string.x_axis_command,
                          String.format(Locale.ENGLISH, "%.2f", xValue)));

        new UdpClient().execute(ipAddr,
                getString(R.string.udp_port),
                getString(R.string.y_axis_command,
                          String.format(Locale.ENGLISH, "%.2f", yValue)));
    }

    @Override
    public void setOnTouchListener(double xValue, double yValue) {
        newData(xValue, yValue, false);
    }

    @Override
    public void setOnMovedListener(double xValue, double yValue) {
        newData(xValue, yValue, false);
    }

    @Override
    public void setOnReleaseListener(double xValue, double yValue) {
        newData(xValue, yValue, true);
    }
}
