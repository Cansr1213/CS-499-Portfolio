// WeightEntry.kt
package com.example.fittrack.data

import androidx.room.Entity
import androidx.room.PrimaryKey

// Defines a Room entity to store weight tracking entries
@Entity(tableName = "weight_entries")
data class WeightEntry(
    @PrimaryKey(autoGenerate = true) val id: Int = 0, // Auto-generated unique ID
    val date: String, // Date of the weight entry
    val weight: String // Recorded weight as a string (e.g., "150")
)
