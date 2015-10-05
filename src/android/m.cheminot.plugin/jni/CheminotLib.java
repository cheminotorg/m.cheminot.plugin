package m.cheminot.plugin.jni;

import java.util.List;

public class CheminotLib {

  static {
    System.loadLibrary("cheminot");
  }

  public static native String gitVersion();

  public static native void load(List<String> graphPaths, List<String> calendarDatesPaths);

  public static native void closeConnection(String dbPath);

  public static native String openConnection(String dbPath);

  public static native String init(String dbPath, List<String> graphPaths, List<String> calendarDatesPaths);

  public static native String lookForBestTrip(String dbPath, String vsId, String veId, int at, int te, int max);

  public static native String lookForBestDirectTrip(String dbPath, String vsId, String veId, int at, int te);

  public static native void abort(String dbPath);

  public static native String trace(String dbPath);

  public static native String getStop(String id);
}
