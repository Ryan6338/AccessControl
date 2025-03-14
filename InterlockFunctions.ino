// Interlock Specific Functions go here.
#ifdef INTERLOCK
void ICACHE_RAM_ATTR checkInSession(String sessionGUID, uint32_t cardNo) {
  log("[SESSION] Session Heartbeat Begin.");
  // Delay to clear wifi buffer.
  delay(10);
  String url;
  if (cardNo > 0) {
    url = String(host) + "/api/" + deviceType + "/session/" + sessionGUID + "/end/" + cardNo + "/?secret=" + String(secret);
  } else {
    url = String(host) + "/api/" + deviceType + "/session/" + sessionGUID + "/heartbeat/?secret=" + String(secret);
  }
  log("[SESSION] Get:" + String(url));
  std::unique_ptr<BearSSL::WiFiClientSecure>SSLclient(new BearSSL::WiFiClientSecure);
  SSLclient->setInsecure();
  client.begin(*SSLclient, url);

  // Start http request.
  int httpCode = client.GET();
  // httpCode will be negative on error
  if (httpCode > 0) {
    // log("[SESSION] Code: " + String(httpCode));

    // Checkin succeeded.
    if (httpCode == HTTP_CODE_OK) {
      String payload = client.getString();
      log("[SESSION] Heartbeat response: " + payload);
    }
  } else {
    log("[SESSION] Heartbeat Error: " + client.errorToString(httpCode));
    statusLight('y');
  }
  client.end();
  log("[SESSION] Session Heartbeat Done.");
  triggerFlag = 0;
  delay(10);
}
void ICACHE_RAM_ATTR activeHeartBeatFlag() {
  triggerFlag = 2;
}
void handleCard(long cardId) {
  // This function handles the handling of the current interlock state vs RFID read.
  if (cardId != lastId) {
    if (!contact) {
      log("[AUTH] Tag is new, checking authentication.");
      statusLight('w');
      Serial.println(millis());
      authCard(cardId);
    } else {
      log("[AUTH] This is someone else disabling the interlock.");
      // Turn off contact, detach timer and heartbeat one last time.
      toggleContact();
      if ( useLocal == 0 ) {
        heartbeatSession.detach();
        checkInSession(sessionID, cardId);
      }
      // Update the user that swipe timeout has begun.
      statusLight('w');
      lastId = 0;
      // Clear temp globals.
      sessionID = "";
    }
  } else {
    log("[AUTH] This is the last user disabling the interlock.");
    // Turn off contact, detach timer and heartbeat one last time.
    toggleContact();
    if ( useLocal == 0 ) {
      heartbeatSession.detach();
      checkInSession(sessionID, cardId);
    }
    // Update the user that swipe timeout has begun.
    statusLight('w');
    lastId = 0;
    // Clear temp globals.
    sessionID = "";
  }
  lastReadSuccess = millis();
}

void statusLight(char color) {
  if (currentColor == color) {
    return;
  } else {
    switch (color) {
      case 'r':
        {
          pixel.setPixelColor(1, pixel.Color(255, 0, 0));
          break;
        }
      case 'g':
        {
          pixel.setPixelColor(1, pixel.Color(0, 255, 0));
          break;
        }
      case 'b':
        {
          pixel.setPixelColor(1, pixel.Color(0, 0, 255));
          break;
        }
      case 'y':
        {
          pixel.setPixelColor(1, pixel.Color(255, 100, 0));
          break;
        }
      case 'p':
        {
          pixel.setPixelColor(1, pixel.Color(128, 0, 128));
          break;
        }
      case 'w':
        {
          pixel.setPixelColor(1, pixel.Color(255, 255, 255));
          break;
        }
    }
    currentColor = color;
    pixel.show();
  }
}
void toggleContact() {
  switch (contact) {
    case 0:
      {
        contact = 1;
        digitalWrite(switchPin, HIGH);
        statusLight('w');
        break;
      }
    case 1:
      {
        contact = 0;
        digitalWrite(switchPin, LOW);
        statusLight('b');
        break;
      }
  }
}
void authCard(long tagid) {
  if (useLocal != 1) {
    log("[AUTH] Server auth check begin");
    String url = String(host) + "/api/" + deviceType + "/check/" + String(tagid) + "/?secret=" + String(secret);
    log("[AUTH] Get:" + String(url));
    std::unique_ptr<BearSSL::WiFiClientSecure>SSLclient(new BearSSL::WiFiClientSecure);
    SSLclient->setInsecure();
    client.begin(*SSLclient, url);

    // Start http request.
    int httpCode = client.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      log("[AUTH] Code: " + String(httpCode));

      // Checkin succeeded.
      if (httpCode == HTTP_CODE_OK) {
        String payload = client.getString();
        log("[AUTH] Server response: " + payload);
        const size_t capacity = JSON_OBJECT_SIZE(4) + 90;
        DynamicJsonDocument doc(capacity);
        deserializeJson(doc, payload);
        if ( doc["access"].as<String>() == "true" ) {
          log("[AUTH] Access granted.");
          sessionID = doc["session_id"].as<String>();
          toggleContact();
          lastId = tagid;
          heartbeatSession.attach(sessionCheckinRate, activeHeartBeatFlag);
        } else {
          log("[AUTH] Access not granted.");
          statusLight('r');
          delay(1000);
        }
        // Clear the json object now.
        doc.clear();

      }
    } else {
      log("[AUTH] Error: " + client.errorToString(httpCode));
      statusLight('y');
    }
    client.end();
    log("[AUTH] Card Auth done.");
    delay(10);
  } else {
    if (tagsArray[0] > 0) {
      log("[AUTH] Checking local cache.");
      for (uint8_t i = 0; i < sizeof(tagsArray); i++) {
        if (tagsArray[i] == tagid) {
          log("[AUTH] Cache Access granted.");
          sessionID = "Cache in use";
          toggleContact();
          lastId = tagid;
          // Server is down, don't bother with heartbeats.
          // heartbeatSession.attach(sessionCheckinRate, activeHeartBeatFlag);
          break;
        } else {
          if (tagsArray[i] < 1) {
            // Check to see if this is the last tag in the cache, if so error.
            log("[AUTH] Cached Access not granted.");
            statusLight('r');
            delay(1000);
            break;
          }
        }
      }
    } else {
      log("[AUTH] Cache check fail, no tags loaded");
    }
  }
}
void checkStateMachine() {
  // Check to see if any of our state flags have tripped.
  switch (triggerFlag) {
    case 1: // Standard heartbeat with server.
      {
        delay(10);
        checkIn();
        delay(10);
        log("[DEBUG] Free Heap Size: " + String(ESP.getFreeHeap()));
        break;
      }
    case 2: // Session heartbeat for interlocks.
      {
        delay(10);
        checkInSession(sessionID, 0);
        delay(10);
        log("[DEBUG] Free Heap Size: " + String(ESP.getFreeHeap()));
        break;
      }
    case 3: // Our cache is out of date, go get it.
      {
        delay(10);
        getCache();
        delay(10);
        triggerFlag = 0;
        break;
      }
    case 4: // If we reach this state, a heartbeat has failed. Load tags, mark as loaded and tell system to use local for next auths.
      {
        if (useLocal < 1) {

          log("[CACHE] Loading local cache.");
          loadTags();
          delay(10);
          tagsLoaded = 1;
          useLocal = 1;
          triggerFlag = 0;
          break;
        } else {
          log("[CACHE] Heartbeat still failed.  ");
          triggerFlag = 0;
          break;
        }
      }
    case 5: // If we reach this state, a heartbeat has succeeded after being in local mode.
      {
        log("[CACHE] Unloading local cache.");
        clearTags();
        delay(10);
        triggerFlag = 0;
        break;
      }
  }
}
#endif
