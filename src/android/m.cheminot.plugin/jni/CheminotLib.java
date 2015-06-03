package m.cheminot.plugin.jni;


public class CheminotLib {

  static {
    System.loadLibrary("cheminot");
  }

  public static native void load(String graphPath, String calendarDatesPath);

  public static native String openConnection(String dbPath);

  public static native String init(String dbPath, String graphPath, String calendarDatesPath);

  public static native String lookForBestTrip(String dbPath, String vsId, String veId, int at, int te, int max);

  public static native String lookForBestDirectTrip(String dbPath, String vsId, String veId, int at, int te);

  public static native void abort(String dbPath);

  public static native String trace(String dbPath);

  public static native String getStop(String id);
}
