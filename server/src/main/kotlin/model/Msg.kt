package com.example.model

import kotlinx.serialization.Serializable
import kotlinx.serialization.json.JsonElement

@Serializable
data class ServerMsg (
    val type: String,
    val value: String
)

@Serializable
data class ClientMsg (
    val type: String,
    val to: String?,
    val from: String,
    val sdp: JsonElement? = null,
    val candidate: JsonElement? = null //ice candidate
)