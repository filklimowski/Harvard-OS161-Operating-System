/**
 * Created by idrees on 2015-02-04.
 */
public class OpponentClient extends RemoteClient {
    int playerId;

    boolean added;

    public OpponentClient(String name, int playerId) {
        super(name);
        this.added = false;
        this.playerId = playerId;
    }

}
