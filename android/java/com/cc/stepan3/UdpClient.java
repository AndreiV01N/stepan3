package com.cc.stepan3;

import android.os.AsyncTask;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;


public class UdpClient extends AsyncTask<String, Void, Void> {

    @Override
    protected Void doInBackground(String... params) {
        String ip_str = params[0];
        String port_str = params[1];
        String msg_str = params[2];

        try (DatagramSocket ds = new DatagramSocket()) {
            DatagramPacket dp;
            byte[] msg = msg_str.getBytes();
            int msg_length = msg_str.length();
            InetAddress ip = InetAddress.getByName(ip_str);
            int port = Integer.parseInt(port_str);
            dp = new DatagramPacket(msg, msg_length, ip, port);
            ds.send(dp);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void result) {
        super.onPostExecute(result);
    }
}
