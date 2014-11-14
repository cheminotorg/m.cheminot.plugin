#include <jni.h>
#include <android/log.h>
#include <string>
#include <map>
#include <list>
#include <memory>
#include <sqlite3.h>
#include <ctime>
#include <cheminotc.h>

std::map<std::string, cheminotc::Vertice> graph;
std::map<std::string, std::list<cheminotc::CalendarException> > calendarExceptions;
sqlite3* connection = NULL;

extern "C" {
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_init(JNIEnv *env, jobject obj, jstring dbpath);
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_lookForBestTrip(JNIEnv *env, jobject obj, jstring startId, jstring endId, jint when);
};

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_init(JNIEnv *env, jobject obj, jstring dbpath) {
  const char *path = env->GetStringUTFChars(dbpath, (jboolean *)0);
  connection = cheminotc::openConnection(path);
  std::string result = cheminotc::getVersion(connection);
  env->ReleaseStringUTFChars(dbpath, path);
  if(calendarExceptions.empty()) {
    calendarExceptions = cheminotc::getCalendarExceptions(connection);
  }
  if(graph.empty()) {
    graph = cheminotc::buildGraph(connection);
  }
  long unsigned int a = graph.size();
  __android_log_print(ANDROID_LOG_DEBUG, "CheminotLog", "GRAPH BUILT %lu", a);
  return env->NewStringUTF(result.c_str());
}

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_lookForBestTrip(JNIEnv *env, jobject obj, jstring startId, jstring endId, jint when) {
  const char* vsId = env->GetStringUTFChars(startId, (jboolean *)0);
  const char* veId = env->GetStringUTFChars(endId, (jboolean *)0);
  struct tm at = cheminotc::asDateTime((int)when);
  long unsigned int a = graph.size();
  long unsigned int b = calendarExceptions.size();
  __android_log_print(ANDROID_LOG_DEBUG, "CheminotLog", "GRAPH %lu", a);
  __android_log_print(ANDROID_LOG_DEBUG, "CheminotLog", "CALENDAR %lu", b);
  __android_log_print(ANDROID_LOG_DEBUG, "CheminotLog", "######> lookForBestTrip %s %s %s", vsId, veId, cheminotc::formatTime(at).c_str());
  std::list<cheminotc::ArrivalTime> arrivalTimes = cheminotc::lookForBestTrip(connection, &graph, &calendarExceptions, vsId, veId, at);
  long unsigned int c = arrivalTimes.size();
  __android_log_print(ANDROID_LOG_DEBUG, "CheminotLog", "######> DONE %lu", c);
  Json::Value serialized = cheminotc::serializeArrivalTimes(arrivalTimes);
  Json::FastWriter* writer = new Json::FastWriter();
  __android_log_print(ANDROID_LOG_DEBUG, "CheminotLog", "######> RESULT %i %s", serialized.type() == Json::ValueType::arrayValue, writer->write(serialized).c_str());
  return env->NewStringUTF(writer->write(serialized).c_str());
}
