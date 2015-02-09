/*
Copyright (C) 2004 Geoffrey Alan Washburn
      
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
      
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
      
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
USA.
*/

import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.ArrayList;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * An implementation of {@link LocalClient} that is controlled by the keyboard
 * of the computer on which the game is being run.  
 * @author Geoffrey Washburn &lt;<a href="mailto:geoffw@cis.upenn.edu">geoffw@cis.upenn.edu</a>&gt;
 * @version $Id: GUIClient.java 343 2004-01-24 03:43:45Z geoffw $
 */

public class GUIClient extends LocalClient implements KeyListener {

        private Socket comm;
        private ObjectOutputStream out;
        private ObjectInputStream in;
        private LinkedBlockingQueue<MazeWarPkt> eventQ;
        private int playerId = (int)(Math.random() * 32000);

        private ArrayList<OpponentClient> opponentList = new ArrayList<OpponentClient>();
        /**
         *
         *
         * Create a GUI controlled {@link LocalClient}.  
         */
        public GUIClient(String name, Socket comm, final Maze maze) {
            super(name);
            this.comm = comm;
            this.eventQ = new LinkedBlockingQueue<MazeWarPkt>();
	    
            try {
            	this.out = new ObjectOutputStream(comm.getOutputStream());
            	this.in = new ObjectInputStream(comm.getInputStream());
                MazeWarPkt p = new MazeWarPkt(MazeWarPkt.MAZEWAR_SPAWN, playerId, name);
                out.writeObject(p);
            }
            catch (IOException err){
                System.out.println("Exception occurred");
            }
            
            //start a new thread to read in broadcast messages from the server
            Thread read = new Thread(){
                public void run(){
                    while(true){
                        try{
				MazeWarPkt packetFromServer = (MazeWarPkt) in.readObject();
				eventQ.put(packetFromServer);
				System.out.println("Received from Server: " + 
				packetFromServer.event + " from Player: " + packetFromServer.player);

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
            
            //another thread to process the head of the queue
            Thread processEvent = new Thread() {
                public void run(){
                    while(true){
                        try{
                            MazeWarPkt eventPkt = eventQ.take();

                            if (eventPkt.player == playerId) {

                                //based on the event, do this action
                                System.out.println("Processing: " + eventPkt.event + " from Player: " + eventPkt.player);

                                if (eventPkt.event == MazeWarPkt.MAZEWAR_CLIENT_LIST_RESP) {
                                    //receiving ArrayList<ClientInfo> clientList;
                                	//clientList contains all clients other than you, with their id,name,x,y,direction
                                	//should do maze.addclient(opponent, x,y, dir) as given in the packet.clientList 
                                	//to add all existing opponents, then do maze.addclient(myself) AFTER so the spawn  
                                	//is indifferent location than any of the existing clients
                                }
                                
                                if (eventPkt.event == MazeWarPkt.MAZEWAR_FORWARD) {
                                    forward();
                                }
                                else if (eventPkt.event == MazeWarPkt.MAZEWAR_BACKWARD) {
                                    backup();
                                }
                                else if (eventPkt.event == MazeWarPkt.MAZEWAR_LEFT) {
                                    turnLeft();
                                }
                                else if (eventPkt.event == MazeWarPkt.MAZEWAR_RIGHT) {
                                    turnRight();
                                }
                                else if (eventPkt.event == MazeWarPkt.MAZEWAR_FIRE) {
                                    fire();
                                }
                            }
                            else {
                                if (eventPkt.event == MazeWarPkt.MAZEWAR_SPAWN) {
                                    OpponentClient opp = new OpponentClient(eventPkt.playerName, eventPkt.player);
                                    opponentList.add(opp);
                                }
                                else if (eventPkt.event == MazeWarPkt.MAZEWAR_COORDINATES) {
                                    for (OpponentClient opponent : opponentList)
                                        if (opponent.playerId == eventPkt.player && !opponent.added) {
                                            opponent.added = true;
                                            maze.addClient(opponent, eventPkt.spawnX, eventPkt.spawnY, eventPkt.spawnD);
                                        }
                                }
                                else if (eventPkt.event == MazeWarPkt.MAZEWAR_CLIENT_LIST_REQ) {
                                    sendCoordinates();
                                }
                                findRemoteClient(eventPkt);
                            }
                            
                            
                        }
                        catch(InterruptedException e){
                            e.printStackTrace();
                        }
                    }
                }
            };

            processEvent.start();
        }

        public void sendCoordinates() {
            MazeWarPkt p = new MazeWarPkt(MazeWarPkt.MAZEWAR_COORDINATES, playerId, this.getName(),
                    this.getPoint().getX(), this.getPoint().getY(), this.getOrientation());
            try {
                out.writeObject(p);
            }
            catch (IOException e) {
                e.printStackTrace();
            }
        }

        public void reqOpponents() {
            MazeWarPkt p = new MazeWarPkt(MazeWarPkt.MAZEWAR_CLIENT_LIST_REQ, playerId, this.getName());
            try {
                out.writeObject(p);
            }
            catch (IOException e) {
                e.printStackTrace();
            }
        }

        public void findRemoteClient(MazeWarPkt packet) {
            for (OpponentClient opponent : opponentList) {
                if (opponent.playerId == packet.player) {
                    if (packet.event == MazeWarPkt.MAZEWAR_FORWARD) {
                        opponent.forward();
                    }
                    else if (packet.event == MazeWarPkt.MAZEWAR_BACKWARD) {
                        opponent.backup();
                    }
                    else if (packet.event == MazeWarPkt.MAZEWAR_LEFT) {
                        opponent.turnLeft();
                    }
                    else if (packet.event == MazeWarPkt.MAZEWAR_RIGHT) {
                        opponent.turnRight();
                    }
                    else if (packet.event == MazeWarPkt.MAZEWAR_FIRE) {
                        opponent.fire();
                    }
                }
            }
        }

        /**
         * Handle a key press.
         * @param e The {@link KeyEvent} that occurred.
         */
        public void keyPressed(KeyEvent e) {
                // If the user pressed Q, invoke the cleanup code and quit.
            try{
                if((e.getKeyChar() == 'q') || (e.getKeyChar() == 'Q')) {
                        //Mazewar.quit();
                    //out = new ObjectOutputStream(comm.getOutputStream());
                    MazeWarPkt p = new MazeWarPkt(MazeWarPkt.MAZEWAR_QUIT, playerId, this.getName());
                    out.writeObject(p);

                    //send a MazeWarPkt with MAZEWAR_QUIT

                // Up-arrow moves forward.
                } else if(e.getKeyCode() == KeyEvent.VK_UP) {
                        //forward();
                    //out = new ObjectOutputStream(comm.getOutputStream());
                    MazeWarPkt p = new MazeWarPkt(MazeWarPkt.MAZEWAR_FORWARD, playerId, this.getName());
                    out.writeObject(p);
                    //send a MazeWarPkt to the server with MAZEWAR_FORWARD

                // Down-arrow moves backward.
                } else if(e.getKeyCode() == KeyEvent.VK_DOWN) {
                        //backup();
                    //out = new ObjectOutputStream(comm.getOutputStream());
                    MazeWarPkt p = new MazeWarPkt(MazeWarPkt.MAZEWAR_BACKWARD, playerId, this.getName());
                    out.writeObject(p);
                    //send a MazeWarPkt to the server with MAZEWAR_BACKWARD
                        
                // Left-arrow turns left.
                } else if(e.getKeyCode() == KeyEvent.VK_LEFT) {
                        //turnLeft();
                    //out = new ObjectOutputStream(comm.getOutputStream());
                    MazeWarPkt p = new MazeWarPkt(MazeWarPkt.MAZEWAR_LEFT, playerId, this.getName());
                    out.writeObject(p);
                    //send a MazeWarPkt to the server with MAZEWAR_LEFT

                // Right-arrow turns right.
                } else if(e.getKeyCode() == KeyEvent.VK_RIGHT) {
                        //turnRight();
                    //out = new ObjectOutputStream(comm.getOutputStream());
                    MazeWarPkt p = new MazeWarPkt(MazeWarPkt.MAZEWAR_RIGHT, playerId, this.getName());
                    out.writeObject(p);
                    //send a MazeWarPkt to the server with MAZEWAR_RIGHT

                // Spacebar fires.
                } else if(e.getKeyCode() == KeyEvent.VK_SPACE) {
                        //fire();
                    //out = new ObjectOutputStream(comm.getOutputStream());
                    MazeWarPkt p = new MazeWarPkt(MazeWarPkt.MAZEWAR_FIRE, playerId, this.getName());
                    out.writeObject(p);
                    //send a MazeWarPkt to the server with MAZEWAR_FIRE
                }
            }
            catch (IOException err) {
                System.out.println("Exception occurred");
            }
        }
        
        /**
         * Handle a key release. Not needed by {@link GUIClient}.
         * @param e The {@link KeyEvent} that occurred.
         */
        public void keyReleased(KeyEvent e) {
        }
        
        /**
         * Handle a key being typed. Not needed by {@link GUIClient}.
         * @param e The {@link KeyEvent} that occurred.
         */
        public void keyTyped(KeyEvent e) {
        }

}
