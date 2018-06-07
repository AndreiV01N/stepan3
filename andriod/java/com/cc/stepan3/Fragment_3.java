package com.cc.stepan3;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.preference.EditTextPreference;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceFragmentCompat;


public class Fragment_3 extends PreferenceFragmentCompat
                        implements SharedPreferences.OnSharedPreferenceChangeListener {

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.pref);

        SharedPreferences mSharedPrefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

        String defaultVideoURL = getResources().getString(R.string.default_video_url);
        String defaultUDPsocket = getResources().getString(R.string.default_udp_socket);

        String VideoURL = mSharedPrefs.getString(getString(R.string.preference_video_url), defaultVideoURL);
        String UDPsocket = mSharedPrefs.getString(getString(R.string.preference_udp_socket), defaultUDPsocket);

        Preference prefVideoURL = getPreferenceScreen().findPreference(getString(R.string.preference_video_url));
        Preference prefUDPsocket = getPreferenceScreen().findPreference(getString(R.string.preference_udp_socket));

        prefVideoURL.setSummary(VideoURL);
        prefUDPsocket.setSummary(UDPsocket);
    }

    @Override
    public void onResume() {
        super.onResume();
        getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
    }

    @Override
    public void onPause() {
        super.onPause();
        getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences sp, String key) {

        Preference pref = getPreferenceScreen().findPreference(key);

        if (key.equals(getString(R.string.preference_video_url))) {
            _updateSummary((EditTextPreference) pref);
        } else if (key.equals(getString(R.string.preference_udp_socket))) {
            _updateSummary((EditTextPreference) pref);
        }
    }

    private void _updateSummary(EditTextPreference pref) {
        pref.setSummary(pref.getText());
    }
}
