package frontend;

import java.io.IOException;
import java.net.UnixDomainSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.nio.charset.StandardCharsets;

import it.unibo.ai.didattica.competition.tablut.domain.Action;
import it.unibo.ai.didattica.competition.tablut.domain.State;

public class EngineSocket {
	
	//private static final String default_path = "/tmp/tablut_socket";
	
	private SocketChannel socket;
	
	public EngineSocket(String path) throws IOException {
		this.socket = SocketChannel.open(UnixDomainSocketAddress.of(path));
	}
	
	public Action computeAction(State state) throws IOException {
		socket.write(ByteBuffer.wrap(state.toLinearString().getBytes()));
		
		ByteBuffer dest = ByteBuffer.allocate(4);
		socket.read(dest);
		String rawAction = new String(dest.array(), StandardCharsets.UTF_8);
		
		return new Action(rawAction.substring(0, 2), rawAction.substring(2, 4), state.getTurn());
	}
	
	public void destroy() throws IOException {
		socket.write(ByteBuffer.wrap("K".getBytes()));
		socket.close();
	}

}
