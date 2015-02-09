package mazewar;

import java.io.ObjectOutputStream;

public class MazeWarBcastThread extends Thread{
	
	public void run() {
			
			//check the queue for elements continuously
			while(true) {
				if(!MazeWarServer.eventQ.isEmpty()) {
					//queue is not empty, broadcast to all
					
					//remove the head of the queue to send it to clients
					MazeWarPkt packetToClient = MazeWarServer.eventQ.remove();
					//(for(i=0;i<numClients;i++) broadcast to all clients. have to have socket of each conn
					
						/* stream to write to client i */
		            	ObjectOutputStream toClient = new ObjectOutputStream(socket.getOutputStream());
		            	
		            	/* send reply back to client */
	                    toClient.writeObject(packetToClient);
	                    
	                    /* continue checking queue */
	                    continue;
				}
			}
		
	}

}
