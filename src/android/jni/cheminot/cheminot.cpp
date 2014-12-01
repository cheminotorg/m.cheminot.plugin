#include <jni.h>
#include <android/log.h>
#include <string>
#include <map>
#include <list>
#include <memory>
#include <sqlite3.h>
#include <ctime>
#include <cheminotc.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <omp.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"Cheminot", __VA_ARGS__)

static cheminotc::Graph graph;
static cheminotc::CalendarDates calendarDates;
static sqlite3* connection = NULL;

extern "C" {
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_init(JNIEnv *env, jclass clazz, jobject jassetManager, jstring jdbpath);
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_lookForBestTrip(JNIEnv *env, jclass jassetManager, jstring startId, jstring endId, jint when);
};

std::string readAssetFile(JNIEnv *env, AAssetManager* assetManager, const char* file) {
  AAsset* asset = AAssetManager_open(assetManager, file, AASSET_MODE_UNKNOWN);
  long size = AAsset_getLength(asset);
  char* buffer = (char*) malloc (sizeof(char)*size);
  AAsset_read(asset, buffer, size);
  AAsset_close(asset);
  return buffer;
}

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_init(JNIEnv *env, jclass clazz, jobject jassetManager, jstring jdbpath) {
  AAssetManager *assetManager = AAssetManager_fromJava(env, jassetManager);
  const char *dbpath = env->GetStringUTFChars(jdbpath, (jboolean *)0);
  connection = cheminotc::openConnection(dbpath);
  std::string result = cheminotc::getVersion(connection);
  env->ReleaseStringUTFChars(jdbpath, dbpath);
  if(calendarDates.empty()) {
    double startRead = omp_get_wtime();
    LOGD(">>>>>> Starting date_exceptions %f", startRead);
    std::string content = readAssetFile(env, assetManager, "calendar_dates");
    double stopRead = omp_get_wtime();
    LOGD(">>>>>> date_exceptions %f", stopRead - startRead);
    double startParse = omp_get_wtime();
    cheminotc::parseCalendarDates(content, &calendarDates);
    double stopParse = omp_get_wtime();
    LOGD(">>>>>> date_exceptions %f", stopParse - startParse);
  }
  if(graph.empty()) {
    double startRead = omp_get_wtime();
    std::string content = readAssetFile(env, assetManager, "graph");
    double stopRead = omp_get_wtime();
    LOGD(">>>>>> graph %f", stopRead - startRead);
    double startParse = omp_get_wtime();
    cheminotc::parseGraph(content, &graph);
    double stopParse = omp_get_wtime();
    LOGD(">>>>>> graph %f", stopParse - startParse);
    long s = graph.size();
    LOGD(">>>>>> graph size %lu", s);
  }
  long unsigned int a = graph.size();
  return env->NewStringUTF(result.c_str());
}

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_lookForBestTrip(JNIEnv *env, jclass obj, jstring startId, jstring endId, jint when) {
  const char* vsId = env->GetStringUTFChars(startId, (jboolean *)0);
  const char* veId = env->GetStringUTFChars(endId, (jboolean *)0);
  struct tm at = cheminotc::asDateTime((int)when);
  long unsigned int a = graph.size();
  long unsigned int b = calendarDates.size();
  LOGD("GRAPH %lu", a);
  LOGD("CALENDAR %lu", b);
  LOGD("###> lookForBestTrip %s %s %s", vsId, veId, cheminotc::formatTime(at).c_str());
  std::list<cheminotc::ArrivalTime> arrivalTimes = cheminotc::lookForBestTrip(connection, &graph, &calendarDates, vsId, veId, at);
  long unsigned int c = arrivalTimes.size();
  LOGD("######> DONE %lu", c);
  Json::Value serialized = cheminotc::serializeArrivalTimes(arrivalTimes);
  Json::FastWriter* writer = new Json::FastWriter();
  return env->NewStringUTF(writer->write(serialized).c_str());
}
