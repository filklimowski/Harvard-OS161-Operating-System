import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

public class MazeWarServer {
	
	private LinkedBlockingQueue<MazeWarPkt> eventQ;
	private ArrayList<ConnectionToClient> clientList;
	private ServerSocket serverSocket;

	public MazeWarServer(int port) {
		
		//Error checking?

		clientList = new ArrayList<ConnectionToClient>();
        eventQ = new LinkedBlockingQueue<MazeWarPkt>();
		try {
			serverSocket = new ServerSocket(port);
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
		//boolean listening = true;
		/*Thread missileTick = new Thread() {
            public void run(){
            	MazeWarPkt tickPkt = new MazeWarPkt(200, -1, "Server");
            	while(true) {
                	try{
                        eventQ.put(tickPkt);
                        Thread.sleep(200);
                	}
                	catch (InterruptedException e) {
                		e.printStackTrace();
                	}
            	}
              }
		};
		
		missileTick.setDaemon(true);
        missileTick.start();*/
            	
		
		Thread accept = new Thread() {
            public void run(){
                while(true){
                    try{
                        Socket s = serverSocket.accept();
                        System.out.println("Accepted connection\n");
			
                        clientList.add(new ConnectionToClient(s));
                    }
                    catch(IOException e){ e.printStackTrace(); }
                }
            }
        };
        
        accept.start();

        Thread broadcastMessages = new Thread() {
            public void run(){
                while(true){
                    try{
                        MazeWarPkt eventPkt = eventQ.take();
                        
                        if(eventPkt.event == MazeWarPkt.MAZEWAR_CLIENT_LIST_REQ) {
                        	//send clientList
                        	int requestingClient=-1;
                        	MazeWarPkt clientListPkt = new MazeWarPkt(MazeWarPkt.MAZEWAR_CLIENT_LIST_RESP, eventPkt.player, eventPkt.playerName);
                        	clientListPkt.clientList = new ArrayList<ClientInfo>();
                        	for(int i=0; i<clientList.size(); i++) {
                        		ConnectionToClient client = clientList.get(i);
                        		if(client.player != eventPkt.player) { //we want to add to list all clients other than the one requesting
                                	ClientInfo clientInfo = new ClientInfo(client.player, client.playerName, client.spawnX, client.spawnY, client.spawnD);
                                	clientListPkt.clientList.add(clientInfo);
                        		}
                        		else {
                        			requestingClient = i;
                        		}
                        	}
                        	if(!clientListPkt.clientList.isEmpty()) { //if empty, requester is only client.
                        		System.out.println("ClientList has playerid: " + clientListPkt.clientList.get(0).player + "\n");
                        		sendToOne(requestingClient, clientListPkt);
                        	}
                        }
                        else {
                        	// dequeue and send to all clients
                            sendToAll(eventPkt);
                            System.out.println("Broadcasting: " + eventPkt.event + " from Player: " + eventPkt.player);
                        }
                    }
                    catch(InterruptedException e){
                        e.printStackTrace();
                    }
                }
            }
        };

        broadcastMessages.start();
    }

    private class ConnectionToClient {

	private Socket socket = null;
	private ObjectInputStream fromClient = null;
	private ObjectOutputStream toClient = null;
	
	private int player = 0;
    private String playerName = null;
    
    private int spawnX = 0;
    private int spawnY = 0;
    private Direction spawnD = null;
    

        ConnectionToClient(Socket socket) throws IOException {
        	
            this.socket = socket;
		    this.fromClient = new ObjectInputStream(socket.getInputStream());
		    /* stream to write back to client */
		    this.toClient = new ObjectOutputStream(socket.getOutputStream());
		    
		    final ConnectionToClient thisClient = this;
		    
            Thread read = new Thread(){
                public void run(){
                    while(true){
                        try{
                            MazeWarPkt packetFromClient = (MazeWarPkt) fromClient.readObject();
                            eventQ.put(packetFromClient);
                            System.out.println("Received and Enqueued " +
                            packetFromClient.event + " from Player: " + packetFromClient.player);
                            
                            if(packetFromClient.event == MazeWarPkt.MAZEWAR_SPAWN) {
                            	thisClient.player = packetFromClient.player;
                            	System.out.println("connection to client playerid is: " + thisClient.player);
                            	thisClient.playerName = packetFromClient.playerName;
                            }
                            else if (packetFromClient.event == MazeWarPkt.MAZEWAR_COORDINATES) {
                            	thisClient.spawnX = packetFromClient.spawnX;
                            	thisClient.spawnY = packetFromClient.spawnY;
                            	thisClient.spawnD = packetFromClient.spawnD;
                            }

                        } catch (ClassNotFoundException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        } catch (InterruptedException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        } catch(IOException e){
                            e.printStackTrace();
                        }
                    }
                }
            };

            read.start();
        }

        public void write(MazeWarPkt eventPkt) {
            try{
                toClient.writeObject(eventPkt);
		System.out.println("Sent back to client\n");
            }
            catch(IOException e){ e.printStackTrace(); }
        }
    }

    public void sendToOne(int index, MazeWarPkt eventPkt) throws IndexOutOfBoundsException {
        clientList.get(index).write(eventPkt);
    }

    public void sendToAll(MazeWarPkt eventPkt){
        for(ConnectionToClient client : clientList)
            client.write(eventPkt);
    }

    public static void main(String args[]) {

        int port = 4444;

        if(args.length == 1 ) {
            port = Integer.parseInt(args[0]);
        } else {
            System.err.println("ERROR: Invalid arguments!");
            System.exit(-1);
        }

		/* Create the GUI */
        new MazeWarServer(port);
    }

}


