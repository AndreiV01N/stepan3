package com.cc.stepan3;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
// import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import java.util.Locale;


public class Fragment_2 extends Fragment {
    private SharedPreferences mSharedPrefs;
    private String ipAddr;
    private String udpPort;
/*
    Position Kp:   |0.08|   .. 0.2       max=20 divider=100
    Position Kd:   |0.20|   .. 0.4       max=20 divider=50
    Speed Kp:      |0.06|   .. 0.08      max=20 divider=250
    Speed Ki:      |0.00|   .. 0.1       max=20 divider=200
    Stability Kp:  |0.30|   .. 1.0       max=20 divider=20
    Stability Kd:  |0.10|   .. 0.5       max=20 divider=40
*/
    int Pos_Kp_default_progress = 8;        // 0.08
    int Pos_Kd_default_progress = 10;       // 0.2

    int Spd_Kp_default_progress = 15;       // 0.06
    int Spd_Ki_default_progress = 16;       // 0.08

    int Stb_Kp_default_progress = 6;        // 0.3
    int Stb_Kd_default_progress = 4;        // 0.1

    int Pos_Kp_divider = 100;
    int Pos_Kd_divider = 50;

    int Spd_Kp_divider = 250;
    int Spd_Ki_divider = 200;

    int Stb_Kp_divider = 20;
    int Stb_Kd_divider = 40;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mSharedPrefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_2, container, false);

        String defaultUDPsocket = getResources().getString(R.string.default_udp_socket);
        String udpSocket = mSharedPrefs.getString(getString(R.string.preference_udp_socket), defaultUDPsocket);
        ipAddr = udpSocket.split(":")[0];
        udpPort = udpSocket.split(":")[1];

        final TextView Pos_Kp_txt_val = rootView.findViewById(R.id.frgm_2_pos_kp_text_val);
        final TextView Pos_Kd_txt_val = rootView.findViewById(R.id.frgm_2_pos_kd_text_val);
        final TextView Spd_Kp_txt_val = rootView.findViewById(R.id.frgm_2_spd_kp_text_val);
        final TextView Spd_Ki_txt_val = rootView.findViewById(R.id.frgm_2_spd_ki_text_val);
        final TextView Stb_Kp_txt_val = rootView.findViewById(R.id.frgm_2_stab_kp_text_val);
        final TextView Stb_Kd_txt_val = rootView.findViewById(R.id.frgm_2_stab_kd_text_val);

        final SeekBar Pos_Kp_sb = rootView.findViewById(R.id.frgm_2_pos_kp_bar);
        final SeekBar Pos_Kd_sb = rootView.findViewById(R.id.frgm_2_pos_kd_bar);
        final SeekBar Spd_Kp_sb = rootView.findViewById(R.id.frgm_2_spd_kp_bar);
        final SeekBar Spd_Ki_sb = rootView.findViewById(R.id.frgm_2_spd_ki_bar);
        final SeekBar Stb_Kp_sb = rootView.findViewById(R.id.frgm_2_stab_kp_bar);
        final SeekBar Stb_Kd_sb = rootView.findViewById(R.id.frgm_2_stab_kd_bar);

        Button Pos_Kp_default_btn = rootView.findViewById(R.id.frgm_2_pos_kp_default);
        Button Pos_Kd_default_btn = rootView.findViewById(R.id.frgm_2_pos_kd_default);
        Button Spd_Kp_default_btn = rootView.findViewById(R.id.frgm_2_spd_kp_default);
        Button Spd_Ki_default_btn = rootView.findViewById(R.id.frgm_2_spd_ki_default);
        Button Stb_Kp_default_btn = rootView.findViewById(R.id.frgm_2_stab_kp_default);
        Button Stb_Kd_default_btn = rootView.findViewById(R.id.frgm_2_stab_kd_default);

        SetToDefault(Pos_Kp_sb, Pos_Kp_txt_val, Pos_Kp_default_progress, Pos_Kp_divider, R.string.pos_kp_command);
        SetToDefault(Pos_Kd_sb, Pos_Kd_txt_val, Pos_Kd_default_progress, Pos_Kd_divider, R.string.pos_kd_command);
        SetToDefault(Spd_Kp_sb, Spd_Kp_txt_val, Spd_Kp_default_progress, Spd_Kp_divider, R.string.spd_kp_command);
        SetToDefault(Spd_Ki_sb, Spd_Ki_txt_val, Spd_Ki_default_progress, Spd_Ki_divider, R.string.spd_ki_command);
        SetToDefault(Stb_Kp_sb, Stb_Kp_txt_val, Stb_Kp_default_progress, Stb_Kp_divider, R.string.stb_kp_command);
        SetToDefault(Stb_Kd_sb, Stb_Kd_txt_val, Stb_Kd_default_progress, Stb_Kd_divider, R.string.stb_kd_command);

        Pos_Kp_default_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                SetToDefault(Pos_Kp_sb, Pos_Kp_txt_val, Pos_Kp_default_progress, Pos_Kp_divider, R.string.pos_kp_command);
            }
        });

        Pos_Kd_default_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                SetToDefault(Pos_Kd_sb, Pos_Kd_txt_val, Pos_Kd_default_progress, Pos_Kd_divider, R.string.pos_kd_command);
            }
        });

        Spd_Kp_default_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                SetToDefault(Spd_Kp_sb, Spd_Kp_txt_val, Spd_Kp_default_progress, Spd_Kp_divider, R.string.spd_kp_command);
            }
        });

        Spd_Ki_default_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                SetToDefault(Spd_Ki_sb, Spd_Ki_txt_val, Spd_Ki_default_progress, Spd_Ki_divider, R.string.spd_ki_command);
            }
        });

        Stb_Kp_default_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                SetToDefault(Stb_Kp_sb, Stb_Kp_txt_val, Stb_Kp_default_progress, Stb_Kp_divider, R.string.stb_kp_command);
            }
        });

        Stb_Kd_default_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View rootView) {
                SetToDefault(Stb_Kd_sb, Stb_Kd_txt_val, Stb_Kd_default_progress, Stb_Kd_divider, R.string.stb_kd_command);
            }
        });

        Pos_Kp_sb.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar sb, int progress, boolean fromUser) {
                ChangeSetting(Pos_Kp_txt_val, progress, Pos_Kp_divider, R.string.pos_kp_command);
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        Pos_Kd_sb.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar sb, int progress, boolean fromUser) {
                ChangeSetting(Pos_Kd_txt_val, progress, Pos_Kd_divider, R.string.pos_kd_command);
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        Spd_Kp_sb.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar sb, int progress, boolean fromUser) {
                ChangeSetting(Spd_Kp_txt_val, progress, Spd_Kp_divider, R.string.spd_kp_command);
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        Spd_Ki_sb.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar sb, int progress, boolean fromUser) {
                ChangeSetting(Spd_Ki_txt_val, progress, Spd_Ki_divider, R.string.spd_ki_command);
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        Stb_Kp_sb.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar sb, int progress, boolean fromUser) {
                ChangeSetting(Stb_Kp_txt_val, progress, Stb_Kp_divider, R.string.stb_kp_command);
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        Stb_Kd_sb.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar sb, int progress, boolean fromUser) {
                ChangeSetting(Stb_Kd_txt_val, progress, Stb_Kd_divider, R.string.stb_kd_command);
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        return rootView;
    }

    public void SetToDefault(SeekBar sb, TextView txt_val, int default_progress, int divider, int resId) {
        String default_value = String.format(Locale.ENGLISH, "%.3f", (float) default_progress / divider);
        txt_val.setText(default_value);
        sb.setProgress(default_progress);
        new UdpClient().execute(ipAddr, udpPort, getString(resId, default_value));
    }

    public void ChangeSetting(TextView txt_val, int progress, int divider, int resId) {
        String current_value = String.format(Locale.ENGLISH, "%.3f", (float) progress / divider);
        txt_val.setText(current_value);
        new UdpClient().execute(ipAddr, udpPort, getString(resId, current_value));
    }
}