package mazewar;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

public class MazeWarThread extends Thread{
	
	private Socket socket = null;
	
	public MazeWarThread(Socket socket) {
		this.socket = socket;
		System.out.println("Created new Thread to handle client");
	}
	
	public void run() {
		
		boolean gotByePacket = false;
		
		try {
			/* stream to read from client */
			ObjectInputStream fromClient = new ObjectInputStream(socket.getInputStream());
			MazeWarPkt packetFromClient;
			
			/* stream to write back to client - will broadcast */
			//ObjectOutputStream toClient = new ObjectOutputStream(socket.getOutputStream());
	
			while (( packetFromClient = (MazeWarPkt) fromClient.readObject()) != null) {
				
				// received an event from a player. 
				//TODO: check that it has proper event&player.id
				
				//Enqueue it, Will broadcast out after 
				MazeWarServer.eventQ.add(packetFromClient);
				
			}
		
		
		
		} catch (IOException e) {
			if(!gotByePacket)
				e.printStackTrace();
		} catch (ClassNotFoundException e) {
			if(!gotByePacket)
				e.printStackTrace();
		}
		
	}

}
