package com.example

import com.example.model.ClientMsg
import com.example.model.ServerMsg
import io.ktor.serialization.kotlinx.KotlinxWebsocketSerializationConverter
import io.ktor.server.application.*
import io.ktor.server.routing.*
import io.ktor.server.websocket.*
import io.ktor.websocket.*
import kotlinx.serialization.json.Json
import java.security.MessageDigest
import java.util.Collections
import kotlin.time.Duration.Companion.seconds
import java.util.UUID


val connections = Collections.synchronizedMap(mutableMapOf<String, DefaultWebSocketServerSession>());

fun generateShortCode(length: Int = 5): String {
    val uuid = UUID.randomUUID().toString()
    val hash = MessageDigest.getInstance("SHA-256")
        .digest(uuid.toByteArray())
        .joinToString("") { "%02x".format(it) }

    return hash.take(length).uppercase()
}


fun Application.configureSockets() {
    install(WebSockets) {
        contentConverter = KotlinxWebsocketSerializationConverter(Json)
        pingPeriod = 15.seconds
        timeout = 15.seconds
        maxFrameSize = Long.MAX_VALUE
        masking = false
    }

    routing {
        webSocket("/ws") {

            val code = generateShortCode();
            connections[code] = this;
            sendSerialized(ServerMsg("init", code));


            try {
                for (frame in incoming) {
                    if (frame is Frame.Text) {
                        val text = frame.readText();
                        val msg = try {
                            Json.decodeFromString<ClientMsg>(text)
                        } catch (e: Exception) {
                            println("Failed to parse message: ${e.message}")
                            sendSerialized(ServerMsg("error", "Invalid message format"))
                            continue
                        }
                        val recepient = connections[msg.to];


                        if (recepient == null) {
                            sendSerialized(ServerMsg("error", "Could not find peer with code ${msg.to}"));
                        } else {
                            recepient.sendSerialized(msg);
                        }
                    }
                }
            } catch (e: Exception) {

            } finally {
                connections.remove(code);
            }
        }
    }
}
