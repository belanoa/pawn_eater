package frontend;

import java.io.IOException;

import it.unibo.ai.didattica.competition.tablut.domain.*;
import it.unibo.ai.didattica.competition.tablut.util.Configuration;

public class Frontend {

	private static final String default_sock_path = "/tmp/tablut_sock";

	public static void main(String[] args) throws IOException {
		// Argument Parsing
		// ServerAddress Color Name
		
		if (args.length != 3 && args.length != 4) {
			System.out.println("USAGE: java -jar frontend.jar <Server Address> <Color> <Name> [Socket Path]");
			System.exit(-1);
		}

		String serverName = args[0];
		String color = args[1];
		String name = args[2];
		String sock_path = args.length == 4 ? args[3] : default_sock_path;

		int port = 0;
		String colorName = "";

		switch (color.toUpperCase()) {
		case "B", "BLACK":
			port = Configuration.blackPort;
			colorName = State.Turn.BLACK.toString();
			break;

		case "W", "WHITE":
			port = Configuration.whitePort;
			colorName = State.Turn.WHITE.toString();
			break;

		default:
			System.exit(-1);
		}

		State curState;
		Action action;

		TablutClientSocket clientSocket = new TablutClientSocket(serverName, port, name);
		System.out.println("Frontend <--> Server OK");
		EngineSocket engineSocket = new EngineSocket(sock_path);
		System.out.println("Frontend <--> Engine OK");

		while (true) {
			curState = clientSocket.readState();

			if (curState.getTurn().equalsTurn(colorName)) {
				System.out.println("Computing action...");
				action = engineSocket.computeAction(curState);
				System.out.println("Action computed, writing result...");
				clientSocket.writeAction(action);
				System.out.println("Result written");
			} else if (!(curState.getTurn().equalsTurn(State.Turn.BLACK.toString()) || curState.getTurn().equalsTurn(State.Turn.WHITE.toString()))) {
				break;
			}

			/*switch (curState.getTurn().toString()) {
				case colorName.toString():
					System.out.println("Computing action...");
					action = engineSocket.computeAction(curState);
					System.out.println("Action computed, writing result...");
					clientSocket.writeAction(action);
					System.out.println("Result written");

				case State.Turn.WHITE.toString(), State.Turn.BLACK.toString():
					continue;
			}

			break;*/
		}

		//System.out.println("DONE, color is " + curState.getTurn().toString());
		
		engineSocket.destroy();
		clientSocket.destroy();

		System.exit(0);
	}

}
