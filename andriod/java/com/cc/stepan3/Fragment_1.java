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
import android.widget.ToggleButton;

import java.util.Locale;


public class Fragment_1 extends Fragment implements JoystickView.OnJoystickChangeListener {
    private TextView xy_pos_text;
    private SharedPreferences mSharedPrefs;
    private String ipAddr;
    private String udpPort;
    private boolean isOnControl = false;
    private boolean isOnAutoroute = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSharedPrefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_1, container, false);

        WebView mWebView = rootView.findViewById(R.id.frgm_1_webview);

        final JoystickView mJoystickView = rootView.findViewById(R.id.frgm_1_joystick);
        mJoystickView.setOnJoystickChangeListener(this);
        if (! isOnControl)
            mJoystickView.setVisibility(View.INVISIBLE);

        xy_pos_text = rootView.findViewById(R.id.frgm_1_text);
        xy_pos_text.setText(R.string.defaultJoystickValue);
        xy_pos_text.setVisibility(View.INVISIBLE);

        String defaultVideoURL = getResources().getString(R.string.default_video_url);
        String videoURL = mSharedPrefs.getString(getString(R.string.preference_video_url), defaultVideoURL);

        String defaultUDPsocket = getResources().getString(R.string.default_udp_socket);
        String udpSocket = mSharedPrefs.getString(getString(R.string.preference_udp_socket), defaultUDPsocket);
        ipAddr = udpSocket.split(":")[0];
        udpPort = udpSocket.split(":")[1];

        mWebView.getSettings().setJavaScriptEnabled(true);
        mWebView.loadUrl(videoURL);

        final Button btnWAKEUP = rootView.findViewById(R.id.frgm_1_wakeup_button);
        final Button btnSLEEP = rootView.findViewById(R.id.frgm_1_sleep_button);

        final ToggleButton btnCTRL = rootView.findViewById(R.id.frgm_1_control_button);
        final Button btnRESET = rootView.findViewById(R.id.frgm_1_reset_button);

        final ToggleButton btnAUTOROUTE = rootView.findViewById(R.id.frgm_1_autoroute_button);
        final Button btnTEST = rootView.findViewById(R.id.frgm_1_test_button);

        if (isOnControl) {
            btnWAKEUP.setEnabled(false);
            btnSLEEP.setEnabled(false);
            btnCTRL.setChecked(true);
            btnCTRL.setEnabled(true);
            btnAUTOROUTE.setChecked(false);
            btnAUTOROUTE.setEnabled(false);
            btnTEST.setEnabled(false);
            xy_pos_text.setVisibility(View.VISIBLE);
            mJoystickView.setVisibility(View.VISIBLE);
        } else if (isOnAutoroute){
            btnWAKEUP.setEnabled(false);
            btnSLEEP.setEnabled(false);
            btnCTRL.setChecked(false);
            btnCTRL.setEnabled(false);
            btnAUTOROUTE.setChecked(true);
            btnAUTOROUTE.setEnabled(true);
            btnTEST.setEnabled(false);
            xy_pos_text.setVisibility(View.INVISIBLE);
            mJoystickView.setVisibility(View.INVISIBLE);
        } else {
            btnWAKEUP.setEnabled(true);
            btnSLEEP.setEnabled(true);
            btnCTRL.setChecked(false);
            btnCTRL.setEnabled(true);
            btnAUTOROUTE.setChecked(false);
            btnAUTOROUTE.setEnabled(true);
            btnTEST.setEnabled(true);
            xy_pos_text.setVisibility(View.INVISIBLE);
            mJoystickView.setVisibility(View.INVISIBLE);
        }

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
                xy_pos_text.setVisibility(View.VISIBLE);
                mJoystickView.setVisibility(View.VISIBLE);
                isOnControl = true;
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
                isOnAutoroute = true;
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
                xy_pos_text.setVisibility(View.INVISIBLE);
                mJoystickView.setVisibility(View.INVISIBLE);
                isOnControl = false;
                isOnAutoroute = false;
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

    private void newData(double xValue, double yValue, boolean isJoystickReleased) {
        if (xValue == 0 && yValue == 0)
            isJoystickReleased = true;

        MyViewPager.setPagingEnabled(isJoystickReleased);

        xy_pos_text.setText(getString(R.string.x_y_label,
                               String.format(Locale.ENGLISH, "%.2f", xValue),
                               String.format(Locale.ENGLISH, "%.2f", yValue)));

        new UdpClient().execute(ipAddr, udpPort,
                                getString(R.string.xy_axis_command,
                                          String.format(Locale.ENGLISH, "%.2f", xValue),
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
