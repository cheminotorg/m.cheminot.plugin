package m.cheminot.plugin;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import m.cheminot.plugin.jni.CheminotLib;

import org.apache.cordova.CallbackContext;
import org.apache.cordova.CordovaPlugin;
import org.json.JSONArray;
import org.json.JSONException;

import android.app.Activity;

public class Cheminot extends CordovaPlugin {

  enum CheminotAction {
    unknown, init, lookForBestTrip
  }

  @Override
  public boolean execute(String action, JSONArray args, CallbackContext cbc) {

    CheminotAction name = CheminotAction.unknown;
    try {
      name = CheminotAction.valueOf(action);
    } catch (IllegalArgumentException e) {}

    switch(name) {

    case init:
      this.init(cbc);
      break;

    case lookForBestTrip:
      this.lookForBestTrip(args, cbc);
      break;

    default:
      cbc.error("Unknown action: " + action);
    }

    return true;
  }

  private void init(final CallbackContext cbc) {
    final String database = "cheminot.db";
    final Activity activity = this.cordova.getActivity();
    this.cordova.getThreadPool().execute(new Runnable() {
        public void run() {
          try {
            prepareDatabase(activity, database);
            String dbpath = activity.getDatabasePath(database).getAbsolutePath();
            cbc.success(CheminotLib.init(activity.getAssets(), dbpath));
          } catch (IOException e) {
            cbc.error(e.getMessage());
          }
        }
    });
  }

  private static void prepareDatabase(Activity activity, String database) throws IOException {
    File dbFile = activity.getDatabasePath(database);
    if(!dbFile.exists()){
      File dbDirectory = new File(dbFile.getParent());
      dbDirectory.mkdirs();
      InputStream in = activity.getApplicationContext().getAssets().open(database);
      OutputStream out = new FileOutputStream(dbFile);
      byte[] buf = new byte[1024];
      int len;
      while ((len = in.read(buf)) > 0) {
        out.write(buf, 0, len);
      }
      in.close();
      out.close();
    }
  }

  private void lookForBestTrip(final JSONArray args, final CallbackContext cbc) {
      this.cordova.getThreadPool().execute(new Runnable() {
          public void run() {
            try {
              String vsId = args.getString(0);
              String veId = args.getString(1);
              int at = args.getInt(2);
              cbc.success(CheminotLib.lookForBestTrip(vsId, veId, at));
            } catch (JSONException e) {
              cbc.error(e.getMessage());
            }
          }
      });
  }
}
