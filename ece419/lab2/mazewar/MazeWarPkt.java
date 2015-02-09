import java.io.Serializable;
import java.util.ArrayList;

class ClientInfo implements Serializable {
	
	public int player;

    public String playerName;

    public int spawnX, spawnY;

    public Direction spawnD;
	
	/* constructor */
	public ClientInfo(int player, String playerName, int spawnX, int spawnY, Direction spawnD) {
		this.player = player;
		this.playerName = playerName;
		this.spawnX = spawnX;
		this.spawnY = spawnY;
		this.spawnD = spawnD;
	}
	
}

public class MazeWarPkt implements Serializable {

	/* define key pressed action */
	public static final int MAZEWAR_FORWARD = 101;
	public static final int MAZEWAR_BACKWARD   = 102;
	public static final int MAZEWAR_LEFT   = 103;
	public static final int MAZEWAR_RIGHT = 104;
	public static final int MAZEWAR_FIRE = 105;
    public static final int MAZEWAR_COORDINATES = 121;
    public static final int MAZEWAR_CLIENT_LIST_REQ = 122;
    public static final int MAZEWAR_CLIENT_LIST_RESP = 123;
	public static final int MAZEWAR_QUIT     = 199;
    public static final int MAZEWAR_SPAWN = 120;
    public static final int MAZEWAR_TICK = 200;
	
	/* error codes */
	
	/* the event is stored here*/
	public int event;
	
	/* the player id */
	public int player;

    public String playerName;

    public int spawnX, spawnY;

    public Direction spawnD;
    
    public ArrayList<ClientInfo> clientList;

    public MazeWarPkt(int event, int player, String playerName) {
        this.event = event;
        this.player = player;
        this.playerName = playerName;
    }

    public MazeWarPkt(int event, int player, String playerName, int spawnX, int spawnY, Direction spawnD) {
        this.event = event;
        this.player = player;
        this.playerName = playerName;
        this.spawnX = spawnX;
        this.spawnY = spawnY;
        this.spawnD = spawnD;
    }

	
}