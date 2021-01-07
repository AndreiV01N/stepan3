package com.cc.stepan3;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
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

    final private float bar_discrete = 20.0f;

    final private float Pos_Kp_default = 0.08f;
    final private float Pos_Kd_default = 0.20f;
    final private float Spd_Kp_default = 0.06f;
    final private float Spd_Ki_default = 0.08f;
    final private float Stb_Kp_default = 0.50f;
    final private float Stb_Kd_default = 0.20f;

    final private float Pos_Kp_divider_f = bar_discrete / (Pos_Kp_default * 2.0f);
    final private float Pos_Kd_divider_f = bar_discrete / (Pos_Kd_default * 2.0f);
    final private float Spd_Kp_divider_f = bar_discrete / (Spd_Kp_default * 2.0f);
    final private float Spd_Ki_divider_f = bar_discrete / (Spd_Ki_default * 2.0f);
    final private float Stb_Kp_divider_f = bar_discrete / (Stb_Kp_default * 2.0f);
    final private float Stb_Kd_divider_f = bar_discrete / (Stb_Kd_default * 2.0f);

    final private int Pos_Kp_dec_base = (int)Math.pow(10, (int)(Math.log10(Pos_Kp_divider_f)));
    final private int Pos_Kd_dec_base = (int)Math.pow(10, (int)(Math.log10(Pos_Kd_divider_f)));
    final private int Spd_Kp_dec_base = (int)Math.pow(10, (int)(Math.log10(Spd_Kp_divider_f)));
    final private int Spd_Ki_dec_base = (int)Math.pow(10, (int)(Math.log10(Spd_Ki_divider_f)));
    final private int Stb_Kp_dec_base = (int)Math.pow(10, (int)(Math.log10(Stb_Kp_divider_f)));
    final private int Stb_Kd_dec_base = (int)Math.pow(10, (int)(Math.log10(Stb_Kd_divider_f)));

    final private int Pos_Kp_divider = Pos_Kp_dec_base * round_to_good_number((int)Math.floor(Pos_Kp_divider_f / Pos_Kp_dec_base));
    final private int Pos_Kd_divider = Pos_Kd_dec_base * round_to_good_number((int)Math.floor(Pos_Kd_divider_f / Pos_Kd_dec_base));
    final private int Spd_Kp_divider = Spd_Kp_dec_base * round_to_good_number((int)Math.floor(Spd_Kp_divider_f / Spd_Kp_dec_base));
    final private int Spd_Ki_divider = Spd_Ki_dec_base * round_to_good_number((int)Math.floor(Spd_Ki_divider_f / Spd_Ki_dec_base));
    final private int Stb_Kp_divider = Stb_Kp_dec_base * round_to_good_number((int)Math.floor(Stb_Kp_divider_f / Stb_Kp_dec_base));
    final private int Stb_Kd_divider = Stb_Kd_dec_base * round_to_good_number((int)Math.floor(Stb_Kd_divider_f / Stb_Kd_dec_base));

    int Pos_Kp_default_progress = (int)(Pos_Kp_default * Pos_Kp_divider);
    int Pos_Kd_default_progress = (int)(Pos_Kd_default * Pos_Kd_divider);
    int Spd_Kp_default_progress = (int)(Spd_Kp_default * Spd_Kp_divider);
    int Spd_Ki_default_progress = (int)(Spd_Ki_default * Spd_Ki_divider);
    int Stb_Kp_default_progress = (int)(Stb_Kp_default * Stb_Kp_divider);
    int Stb_Kd_default_progress = (int)(Stb_Kd_default * Stb_Kd_divider);

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
        sleep_ms(50);
        SetToDefault(Pos_Kd_sb, Pos_Kd_txt_val, Pos_Kd_default_progress, Pos_Kd_divider, R.string.pos_kd_command);
        sleep_ms(50);
        SetToDefault(Spd_Kp_sb, Spd_Kp_txt_val, Spd_Kp_default_progress, Spd_Kp_divider, R.string.spd_kp_command);
        sleep_ms(50);
        SetToDefault(Spd_Ki_sb, Spd_Ki_txt_val, Spd_Ki_default_progress, Spd_Ki_divider, R.string.spd_ki_command);
        sleep_ms(50);
        SetToDefault(Stb_Kp_sb, Stb_Kp_txt_val, Stb_Kp_default_progress, Stb_Kp_divider, R.string.stb_kp_command);
        sleep_ms(50);
        SetToDefault(Stb_Kd_sb, Stb_Kd_txt_val, Stb_Kd_default_progress, Stb_Kd_divider, R.string.stb_kd_command);
        sleep_ms(50);

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

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        super.setUserVisibleHint(isVisibleToUser);
        if (isVisibleToUser) {
            String defaultUDPsocket = getResources().getString(R.string.default_udp_socket);
            String udpSocket = mSharedPrefs.getString(getString(R.string.preference_udp_socket), defaultUDPsocket);
            ipAddr = udpSocket.split(":")[0];
            udpPort = udpSocket.split(":")[1];
        }
    }

    public void SetToDefault(SeekBar sb, TextView txt_val, int default_progress, int divider, int resId) {
        String default_value = String.format(Locale.ENGLISH, "%.3f", (float) default_progress / divider);
        txt_val.setText(default_value);
        sb.setProgress(default_progress);
    }

    public void ChangeSetting(TextView txt_val, int progress, int divider, int resId) {
        String current_value = String.format(Locale.ENGLISH, "%.3f", (float) progress / divider);
        txt_val.setText(current_value);
        new UdpClient().execute(ipAddr, udpPort, getString(resId, current_value));
    }

    private int round_to_good_number(int num) {
        if (num == 3) return 4;
        if (num == 6 || num == 7) return 5;
        if (num == 8 || num == 9) return 10;
        return num;
    }

    private void sleep_ms(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException ie) {
            Thread.currentThread().interrupt();
        }
    }
}
