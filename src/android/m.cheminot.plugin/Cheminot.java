package m.cheminot.plugin;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import m.cheminot.plugin.jni.CheminotLib;

import org.apache.cordova.CallbackContext;
import org.apache.cordova.CordovaPlugin;
import org.json.JSONArray;
import org.json.JSONException;

import android.app.Activity;
import android.content.res.AssetManager;
import android.content.res.Resources;

public class Cheminot extends CordovaPlugin {

  private static String DBPATH;

  enum CheminotAction {
    unknown, gitVersion, init, lookForBestTrip, lookForBestDirectTrip, abort, trace, getStop
  }

  static class Subset {
    private String graph;
    private String calendarDates;

    public Subset() {
    }

    public boolean isValid() {
      return this.graph != null && this.calendarDates != null;
    }

    public String getGraph() {
      return this.graph;
    }

    public String getCalendarDates() {
      return this.calendarDates;
    }

    public void setGraph(String graph) {
      this.graph = graph;
    }

    public void setCalendarDates(String calendarDates) {
      this.calendarDates = calendarDates;
   }
  }

  static class CheminotDB {
    private String db;
    private Subset ter;
    private Subset trans;
    private Date date;

    static Pattern pattern = Pattern.compile("((\\w+)-)?(\\w+)-(\\d+)(\\.db)?");
    static SimpleDateFormat format = new SimpleDateFormat("yyyyMMddHHmmss", Locale.FRANCE);

    public CheminotDB() {
      this.ter = new Subset();
      this.trans = new Subset();
    }

    public boolean isValid() {
      return this.db != null && this.trans.isValid() && this.ter.isValid();
    }

    public boolean isMoreRecent(CheminotDB other) {
      return this.date.getTime() > other.getDate().getTime();
    }

    public String getDb() {
      return this.db;
    }

    public Date getDate() {
      return this.date;
    }

    public Subset getTrans() {
      return this.trans;
    }

    public Subset getTer() {
      return this.ter;
    }

    public void setDb(String db) {
      this.db = db;
    }

    public void setDate(Date date) {
      this.date = date;
    }

    public void setTer(Subset ter) {
      this.ter = ter;
    }

    public void setTrans(Subset trans) {
      this.trans = trans;
    }
  }

  @Override
  public boolean execute(String action, JSONArray args, CallbackContext cbc) {

    CheminotAction name = CheminotAction.unknown;
    try {
      name = CheminotAction.valueOf(action);
    } catch (IllegalArgumentException e) {}

    switch(name) {

    case gitVersion:
      this.gitVersion(cbc);
      break;

    case init:
      this.init(cbc);
      break;

    case lookForBestTrip:
      this.lookForBestTrip(args, cbc);
      break;

    case lookForBestDirectTrip:
      this.lookForBestDirectTrip(args, cbc);
      break;

    case abort:
      this.abort(cbc);
      break;

    case trace:
      this.trace(args, cbc);
      break;

    case getStop:
      this.getStop(args, cbc);

    default:
      cbc.error("Unknown action: " + action);
    }

    return true;
  }

  private void gitVersion(final CallbackContext cbc) {
    cbc.success(CheminotLib.gitVersion());
  }

  private void init(final CallbackContext cbc) {
    final Activity activity = this.cordova.getActivity();
      this.cordova.getThreadPool().execute(new Runnable() {
        public void run() {
          try {
            final CheminotDB cheminotDB = getMostRecentDB(activity);
            if(cheminotDB != null) {
              File dbFile = copyFromAssets(activity, cheminotDB.getDb(), 4096);
              File terGraphFile = copyFromAssets(activity, cheminotDB.getTer().getGraph(), 4096);
              File terCalendarDatesFile = copyFromAssets(activity, cheminotDB.getTer().getCalendarDates(), 1024);
              File transGraphFile = copyFromAssets(activity, cheminotDB.getTrans().getGraph(), 4096);
              File transCalendarDatesFile = copyFromAssets(activity, cheminotDB.getTrans().getCalendarDates(), 1024);
              cleanDbDirectory(new File(dbFile.getParent()), cheminotDB);
              DBPATH = dbFile.getAbsolutePath();

              ArrayList<String> graphPaths = new ArrayList<String>();
              graphPaths.add(terGraphFile.getAbsolutePath());
              //graphPaths.add(transGraphFile.getAbsolutePath());

              ArrayList<String> calendarDatesPaths = new ArrayList<String>();
              calendarDatesPaths.add(terCalendarDatesFile.getAbsolutePath());
              //calendarDatesPaths.add(transCalendarDatesFile.getAbsolutePath());

              android.util.Log.d("Cheminot", terGraphFile.getAbsolutePath());
              android.util.Log.d("Cheminot", transGraphFile.getAbsolutePath());
              android.util.Log.d("Cheminot", terCalendarDatesFile.getAbsolutePath());
              android.util.Log.d("Cheminot", transCalendarDatesFile.getAbsolutePath());
              String result = CheminotLib.init(dbFile.getAbsolutePath(), graphPaths, calendarDatesPaths);
              cbc.success(result);
            } else {
              cbc.error("Unable to find the most recent db");
            }
          } catch (IOException e) {
            cbc.error(e.getMessage());
          }
        }
      });
  }

  private static File copyFromAssets(Activity activity, String file, int bufsize) throws IOException {
    File dbFile = activity.getDatabasePath(file);
    if(!dbFile.exists()) {
      File dbDirectory = new File(dbFile.getParent());
      dbDirectory.mkdirs();
      InputStream in = activity.getApplicationContext().getAssets().open(file);
      OutputStream out = new FileOutputStream(dbFile);
      byte[] buf = new byte[bufsize];
      int len;
      while ((len = in.read(buf)) > 0) {
        out.write(buf, 0, len);
      }
      in.close();
      out.close();
    }
    return activity.getDatabasePath(file);
  }

  private static void cleanDbDirectory(File dbDir, CheminotDB cheminotDB) {
    for(String name : dbDir.list()) {
      File file = new File(dbDir.getAbsolutePath() + "/" + name);
      Matcher matcher = CheminotDB.pattern.matcher(name);
      if(matcher.matches()) {
        String version = matcher.group(4);
        try {
          Date date = CheminotDB.format.parse(version);
          if(date.getTime() < cheminotDB.getDate().getTime()) {
            file.delete();
          }
        } catch (ParseException e) {
          file.delete();
        }
      } else {
        file.delete();
      }
    }
  }

  private static CheminotDB getMostRecentDB(Activity activity) {
    Resources ressources = activity.getResources();
    AssetManager assetManager = ressources.getAssets();
    CheminotDB mostRecentDB = null;

    try {
      String[] files = assetManager.list("");

      Map<String, CheminotDB> dbByVersion = new HashMap<String, CheminotDB>();
      for(String file : files) {
        Matcher matcher = CheminotDB.pattern.matcher(file);
        if(matcher.matches()) {
          String id = matcher.group(2);
          String name = matcher.group(3);
          String version = matcher.group(4);
          android.util.Log.d("cheminot", name);
          try {
            Date date = CheminotDB.format.parse(version);
            if(dbByVersion.get(version) == null) {
              dbByVersion.put(version, new CheminotDB());
            }
            CheminotDB cheminotDB = dbByVersion.get(version);
            cheminotDB.setDate(date);
            if(name.equals("cheminot")) {
              cheminotDB.setDb(file);
            } else if(name.equals("graph")) {
              if(id.equals("ter")) {
                cheminotDB.getTer().setGraph(file);
              } else if(id.equals("trans")) {
                cheminotDB.getTrans().setGraph(file);
              }
            } else if(name.equals("calendardates")) {
              if(id.equals("ter")) {
                cheminotDB.getTer().setCalendarDates(file);
              } else if(id.equals("trans")) {
                cheminotDB.getTrans().setCalendarDates(file);
              }
            }
          } catch (ParseException e) {}
        }
      }

      for(CheminotDB cheminotDB : dbByVersion.values()) {
        if(mostRecentDB == null || cheminotDB.isMoreRecent(mostRecentDB)) {
          if(cheminotDB.isValid()) {
            mostRecentDB = cheminotDB;
          }
        }
      }
    } catch (IOException e) {}

    return mostRecentDB;
  }

  private void lookForBestTrip(final JSONArray args, final CallbackContext cbc) {
      this.cordova.getThreadPool().execute(new Runnable() {
          public void run() {
            try {
              String vsId = args.getString(0);
              String veId = args.getString(1);
              int at = args.getInt(2);
              int te = args.getInt(3);
              int max = args.getInt(4);
              cbc.success(CheminotLib.lookForBestTrip(DBPATH, vsId, veId, at, te, max));
            } catch (JSONException e) {
              cbc.error("Unable to perform `lookForBestTrip`: " + e.getMessage());
            }
          }
      });
  }

  private void lookForBestDirectTrip(final JSONArray args, final CallbackContext cbc) {
      this.cordova.getThreadPool().execute(new Runnable() {
          public void run() {
            try {
              String vsId = args.getString(0);
              String veId = args.getString(1);
              int at = args.getInt(2);
              int te = args.getInt(3);
              cbc.success(CheminotLib.lookForBestDirectTrip(DBPATH, vsId, veId, at, te));
            } catch (JSONException e) {
              cbc.error("Unable to perform `lookForBestDirectTrip`: " + e.getMessage());
            }
          }
      });
  }

  private void abort(final CallbackContext cbc) {
    this.cordova.getThreadPool().execute(new Runnable() {
      public void run() {
        CheminotLib.abort(DBPATH);
        cbc.success();
      }
    });
  }

  private void trace(final JSONArray args, final CallbackContext cbc) {
    this.cordova.getThreadPool().execute(new Runnable() {
      public void run() {
        cbc.success(CheminotLib.trace(DBPATH));
      };
    });
  };

  private void getStop(final JSONArray args, final CallbackContext cbc) {
    this.cordova.getThreadPool().execute(new Runnable() {
      public void run() {
        try {
          String stopId = args.getString(0);
          cbc.success(CheminotLib.getStop(stopId));
        } catch (JSONException e) {
          cbc.error("Unable to perform `getStop`: " + e.getMessage());
        }
      };
    });
  };
}
