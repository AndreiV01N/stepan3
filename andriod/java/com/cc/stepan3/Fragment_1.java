package com.cc.stepan3;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.widget.Button;
import android.widget.TextView;
import android.widget.ToggleButton;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Locale;


public class Fragment_1 extends Fragment implements JoystickView.OnJoystickChangeListener {
    private TextView text;
    private SharedPreferences mSharedPrefs;
    private String ipAddr;
    private String udpPort;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSharedPrefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_1, container, false);

//        WebView mWebView = rootView.findViewById(R.id.frgm_1_webview);

        JoystickView mJoystickView = rootView.findViewById(R.id.frgm_1_joystick);
        mJoystickView.setOnJoystickChangeListener(this);
        MainActivity.joystickReleased = true;

        text = rootView.findViewById(R.id.frgm_1_text);
        text.setText(R.string.defaultJoystickValue);

//        String defaultVideoURL = getResources().getString(R.string.default_video_url);
//        String videoURL = mSharedPrefs.getString(getString(R.string.preference_video_url), defaultVideoURL);

        String defaultUDPsocket = getResources().getString(R.string.default_udp_socket);
        String udpSocket = mSharedPrefs.getString(getString(R.string.preference_udp_socket), defaultUDPsocket);
        ipAddr = udpSocket.split(":")[0];
        udpPort = udpSocket.split(":")[1];

//        mWebView.getSettings().setJavaScriptEnabled(true);
//        mWebView.loadUrl(videoURL);

        final Button btnWAKEUP = rootView.findViewById(R.id.frgm_1_wakeup_button);
        final Button btnSLEEP = rootView.findViewById(R.id.frgm_1_sleep_button);

        final ToggleButton btnCTRL = rootView.findViewById(R.id.frgm_1_control_button);
        Button btnRESET = rootView.findViewById(R.id.frgm_1_reset_button);

        final ToggleButton btnAUTOROUTE = rootView.findViewById(R.id.frgm_1_autoroute_button);
        final Button btnTEST = rootView.findViewById(R.id.frgm_1_test_button);

        btnWAKEUP.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr, udpPort, getString(R.string.wakeup_command, "1"));
            }
        });

        btnSLEEP.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr, udpPort, getString(R.string.sleep_command, "1"));
            }
        });

        btnCTRL.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr, udpPort, getString(R.string.control_command, "1"));
                btnWAKEUP.setEnabled(false);
                btnSLEEP.setEnabled(false);
                btnAUTOROUTE.setEnabled(false);
                btnTEST.setEnabled(false);
                btnCTRL.setChecked(true);
            }
        });

        btnAUTOROUTE.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr, udpPort, getString(R.string.autoroute_command, "1"));
                btnWAKEUP.setEnabled(false);
                btnSLEEP.setEnabled(false);
                btnCTRL.setEnabled(false);
                btnTEST.setEnabled(false);
                btnAUTOROUTE.setChecked(true);
            }
        });

        btnRESET.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr, udpPort, getString(R.string.reset_command, "1"));
                btnWAKEUP.setEnabled(true);
                btnSLEEP.setEnabled(true);
                btnCTRL.setEnabled(true);
                btnAUTOROUTE.setEnabled(true);
                btnTEST.setEnabled(true);
                btnCTRL.setChecked(false);
                btnAUTOROUTE.setChecked(false);
            }
        });

        btnTEST.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                new UdpClient().execute(ipAddr, udpPort, getString(R.string.test_command, "1"));
            }
        });

        return rootView;
    }

    private void newData(double xValue, double yValue, boolean joystickReleased) {
        if (xValue == 0 && yValue == 0)
            joystickReleased = true;

        MyViewPager.setPagingEnabled(joystickReleased);
        MainActivity.joystickReleased = joystickReleased;

        text.setText(getString(R.string.x_y_label,
                               String.format(Locale.ENGLISH, "%.2f", xValue),
                               String.format(Locale.ENGLISH, "%.2f", yValue)));

        new UdpClient().execute(ipAddr, udpPort,
                getString(R.string.x_axis_command, String.format(Locale.ENGLISH, "%.2f", xValue)));

        new UdpClient().execute(ipAddr, udpPort,
                getString(R.string.y_axis_command, String.format(Locale.ENGLISH, "%.2f", yValue)));
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
