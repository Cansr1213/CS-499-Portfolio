// UserDao.kt
package com.example.fittrack.data

import androidx.room.Dao
import androidx.room.Insert
import androidx.room.Query

// Data access object for User entity
@Dao
interface UserDao {
    @Insert
    suspend fun insertUser(user: User) // Inserts a new user

    @Query("""
        SELECT * FROM users 
        WHERE username = :username
        AND passwordHash = :passwordHash
    """)
    suspend fun getUser(username: String, passwordHash: String):User?


    @Query("SELECT * FROM users WHERE username = :username")
    suspend fun getUserByUsername(username: String): User? // Retrieves a user by username
}
