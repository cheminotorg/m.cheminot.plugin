#include <jni.h>
#include <string>
#include <map>
#include <list>
#include <memory>
#include <sqlite3.h>
#include <ctime>
#include <cheminotc.h>
#include <settings.h>

static std::unordered_map<std::string, cheminotc::CheminotDb> connections;

static cheminotc::Graph graph;
static cheminotc::CalendarDates calendarDates;
static cheminotc::Cache cache;

extern "C" {
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_gitVersion(JNIEnv *env, jclass clazz);
  JNIEXPORT void JNICALL Java_m_cheminot_plugin_jni_CheminotLib_load(JNIEnv *env, jclass clazz, jobject jGraphPaths, jobject jCalendarDatesPaths);
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_openConnection(JNIEnv *env, jclass clazz, jstring jdbPath);
  JNIEXPORT void JNICALL Java_m_cheminot_plugin_jni_CheminotLib_closeConnection(JNIEnv *env, jclass clazz, jstring jdbPath);
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_init(JNIEnv *env, jclass clazz, jstring jdbPath, jobject jGraphPaths, jobject jCalendarDatesPaths);
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_lookForBestTrip(JNIEnv *env, jclass clazz, jstring jdbPath, jstring jvsId, jstring jveId, jint jat, jint jte, jint jmax);
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_lookForBestDirectTrip(JNIEnv *env, jclass clazz, jstring jdbPath, jstring jvsId, jstring jveId, jint jat, jint jte);
  JNIEXPORT void JNICALL Java_m_cheminot_plugin_jni_CheminotLib_abort(JNIEnv *env, jclass clazz, jstring jdbPath);
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_trace(JNIEnv *env, jclass clazz, jstring jdbPath);
  JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_getStop(JNIEnv *env, jclass clazz, jstring jstopId);
};

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_gitVersion(JNIEnv *env, jclass clazz) {
  return env->NewStringUTF(cheminotc::gitVersion.c_str());
}

JNIEXPORT void JNICALL Java_m_cheminot_plugin_jni_CheminotLib_closeConnection(JNIEnv *env, jclass clazz, jstring jdbPath) {
  const char *dbPath = env->GetStringUTFChars(jdbPath, (jboolean *)0);

  auto connectionIt = connections.find(dbPath);
  if(connectionIt != connections.end())
  {
    cheminotc::CheminotDb connection = connectionIt->second;
    cheminotc::closeConnection(connection);
    connections.erase(dbPath);
  }
}

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_openConnection(JNIEnv *env, jclass clazz, jstring jdbPath) {
  const char *dbPath = env->GetStringUTFChars(jdbPath, (jboolean *)0);

  cheminotc::CheminotDb connection;
  auto existingConnectionIt = connections.find(dbPath);

  if(existingConnectionIt == connections.end())
  {
    connection = cheminotc::openConnection(dbPath);
    connections[dbPath] = connection;
  }
  else
  {
    connection = existingConnectionIt->second;
  }

  Json::Value meta = cheminotc::getMeta(connection);

  env->ReleaseStringUTFChars(jdbPath, dbPath);

  Json::FastWriter* writer = new Json::FastWriter();
  return env->NewStringUTF(writer->write(meta).c_str());
}

std::list<std::string> jListToList(JNIEnv *env, jobject &jList) {
  jclass cList = env->FindClass("java/util/List");
  jmethodID mToArray = env->GetMethodID(cList, "toArray", "()[Ljava/lang/Object;");
  jobjectArray array = (jobjectArray)env->CallObjectMethod(jList, mToArray);
  std::list<std::string> sArray;
  for(int i=0; i<env->GetArrayLength(array); i++) {
    jstring strObj = (jstring)env->GetObjectArrayElement(array, i);
    const char *chr = env->GetStringUTFChars(strObj, JNI_FALSE);
    sArray.push_back(chr);
    env->ReleaseStringUTFChars(strObj, chr);
  }
  return sArray;
}

JNIEXPORT void JNICALL Java_m_cheminot_plugin_jni_CheminotLib_load(JNIEnv *env, jclass clazz, jobject jGraphPaths, jobject jCalendarDatesPaths) {

  if(graph.empty()) {
    std::list<std::string> graphPaths = jListToList(env, jGraphPaths);
    cheminotc::parseGraphFiles(graphPaths, graph);
  }

  if(calendarDates.empty()) {
    std::list<std::string> calendarDatesPaths = jListToList(env, jCalendarDatesPaths);
    cheminotc::parseCalendarDatesFiles(calendarDatesPaths, calendarDates);
  }

  cheminotc::fillCache(cache, calendarDates, graph);
}

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_init(JNIEnv *env, jclass clazz, jstring jdbPath, jobject jGraphPaths, jobject jCalendarDatesPaths) {
  const char *dbPath = env->GetStringUTFChars(jdbPath, (jboolean *)0);

  cheminotc::CheminotDb connection;
  auto existingConnectionIt = connections.find(dbPath);

  if(existingConnectionIt == connections.end())
  {
    connection = cheminotc::openConnection(dbPath);
    connections[dbPath] = connection;
  }
  else
  {
    connection = existingConnectionIt->second;
  }

  Json::Value meta = cheminotc::getMeta(connection);

  if(graph.empty()) {
    std::list<std::string> graphPaths = jListToList(env, jGraphPaths);
    cheminotc::parseGraphFiles(graphPaths, graph);
  }

  if(calendarDates.empty()) {
    std::list<std::string> calendarDatesPaths = jListToList(env, jCalendarDatesPaths);
    cheminotc::parseCalendarDatesFiles(calendarDatesPaths, calendarDates);
  }

  env->ReleaseStringUTFChars(jdbPath, dbPath);

  Json::FastWriter* writer = new Json::FastWriter();

  return env->NewStringUTF(writer->write(meta).c_str());
}

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_lookForBestTrip(JNIEnv *env, jclass clazz, jstring jdbPath, jstring jvsId, jstring jveId, jint jat, jint jte, jint jmax) {
  const char *dbPath = env->GetStringUTFChars(jdbPath, (jboolean *)0);
  const char* vsId = env->GetStringUTFChars(jvsId, (jboolean *)0);
  const char* veId = env->GetStringUTFChars(jveId, (jboolean *)0);
  tm at = cheminotc::asDateTime((int)jat);
  tm te = cheminotc::asDateTime((int)jte);
  int max = (int)jmax;

  cheminotc::CheminotDb connection = connections[dbPath];

  cheminotc::resetTrace(connection);
  cheminotc::unlock(connection);
  auto result = cheminotc::lookForBestTrip(connection, graph, cache, calendarDates, vsId, veId, at, te, max);
  std::list<cheminotc::ArrivalTime> arrivalTimes = result.second;
  bool locked = result.first;

  env->ReleaseStringUTFChars(jdbPath, dbPath);
  env->ReleaseStringUTFChars(jvsId, vsId);
  env->ReleaseStringUTFChars(jveId, veId);

  cheminotc::resetTrace(connection);

  if(!locked) {
    Json::Value serialized = cheminotc::serializeArrivalTimes(graph, cache, arrivalTimes);
    Json::FastWriter* writer = new Json::FastWriter();
    return env->NewStringUTF(writer->write(serialized).c_str());
  } else {
    return env->NewStringUTF("null");
  }
}

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_lookForBestDirectTrip(JNIEnv *env, jclass clazz, jstring jdbPath, jstring jvsId, jstring jveId, jint jat, jint jte) {
  const char *dbPath = env->GetStringUTFChars(jdbPath, (jboolean *)0);
  const char* vsId = env->GetStringUTFChars(jvsId, (jboolean *)0);
  const char* veId = env->GetStringUTFChars(jveId, (jboolean *)0);
  tm at = cheminotc::asDateTime((int)jat);
  tm te = cheminotc::asDateTime((int)jte);

  cheminotc::CheminotDb connection = connections[dbPath];

  cheminotc::unlock(connection);
  std::pair<bool, std::list<cheminotc::ArrivalTime>> result = lookForBestDirectTrip(connection, { "TER" }, graph, cache, calendarDates, vsId, veId, at, te);
  std::list<cheminotc::ArrivalTime> arrivalTimes = result.second;

  env->ReleaseStringUTFChars(jdbPath, dbPath);
  env->ReleaseStringUTFChars(jvsId, vsId);
  env->ReleaseStringUTFChars(jveId, veId);

  Json::Value serialized = cheminotc::serializeArrivalTimes(graph, cache, arrivalTimes);
  Json::Value json;
  json["arrivalTimes"] = serialized;
  json["hasDirect"] = result.first;
  Json::FastWriter* writer = new Json::FastWriter();
  return env->NewStringUTF(writer->write(json).c_str());
}

JNIEXPORT void JNICALL Java_m_cheminot_plugin_jni_CheminotLib_abort(JNIEnv *env, jclass clazz, jstring jdbPath) {
  const char *dbPath = env->GetStringUTFChars(jdbPath, (jboolean *)0);
  cheminotc::CheminotDb connection = connections[dbPath];
  env->ReleaseStringUTFChars(jdbPath, dbPath);
  cheminotc::lock(connection);
}

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_trace(JNIEnv *env, jclass clazz, jstring jdbPath) {
  const char *dbPath = env->GetStringUTFChars(jdbPath, (jboolean *)0);
  cheminotc::CheminotDb connection = connections[dbPath];
  std::string trace = cheminotc::getLastTrace(connection);
  env->ReleaseStringUTFChars(jdbPath, dbPath);
  return env->NewStringUTF(trace.c_str());
}

JNIEXPORT jstring JNICALL Java_m_cheminot_plugin_jni_CheminotLib_getStop(JNIEnv *env, jclass clazz, jstring jstopId) {
  const char *stopId = env->GetStringUTFChars(jstopId, (jboolean *)0);
  std::string stop = "null";
  if(cheminotc::verticeExists(&graph, &cache, stopId))
  {
      cheminotc::Vertice vertice = cheminotc::getVerticeFromGraph(graph, cache, stopId, nullptr, false);
      Json::Value json;
      json["id"] = vertice.id;
      json["name"] = vertice.name;
      json["lat"] = vertice.lat;
      json["lng"] = vertice.lng;
      env->ReleaseStringUTFChars(jstopId, stopId);
      Json::FastWriter* writer = new Json::FastWriter();
      stop = writer->write(json).c_str();
  }
  return env->NewStringUTF(stop.c_str());
}
