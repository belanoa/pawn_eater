package frontend;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

import com.google.gson.*;

import it.unibo.ai.didattica.competition.tablut.domain.Action;
import it.unibo.ai.didattica.competition.tablut.domain.StateTablut;

public class TablutClientSocket {
	private Socket socket;
	private DataInputStream in;
	private DataOutputStream out;
	private Gson gson;
	
	public TablutClientSocket(String address, int port, String name) throws IOException {
		InetAddress addr = InetAddress.getByName(address);
		
		this.gson = new Gson();
		this.socket = new Socket(addr, port);
		
		in = new DataInputStream(socket.getInputStream());
		out = new DataOutputStream(socket.getOutputStream());

		byte[] nameBytes = name.getBytes(StandardCharsets.UTF_8);
		out.writeInt(nameBytes.length);
		out.write(nameBytes);
	}
	
	public void writeAction(Action action) throws IOException {
		byte[] json = this.gson.toJson(action, Action.class).getBytes();
		
		this.out.writeInt(json.length);
		this.out.write(json);
	}
	
	public StateTablut readState() throws IOException {
		int size = in.readInt();
		byte[] bytes = new byte[size];
		
		in.read(bytes, 0, size);
		
		return this.gson.fromJson(new String(bytes, StandardCharsets.UTF_8), StateTablut.class);
	}
	
	public void destroy() throws IOException {
		this.socket.close();
	}
}
