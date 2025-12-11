// WeightDao.kt
package com.example.fittrack.data

import androidx.room.*
import kotlinx.coroutines.flow.Flow

@Dao
interface WeightDao{

    // Insert weight entry
    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertWeight(entry: WeightEntry)

    // Update weight entry
    @Update
    suspend fun updateWeight(entry: WeightEntry)

    // Delete Weight entry
    @Delete
    suspend fun deleteWeight(entry: WeightEntry)

    // Updated Retrieved only logged-in user's data
    @Query("""
        SELECT * FROM weight_entries
        WHERE userUsername = :username
        ORDER BY date ASC 
    """)
    fun getWeightsForUser(username: String): Flow<List<WeightEntry>>
}