import { Database } from "sqlite3";
import { mkdir } from "node:fs/promises";
import { dirname } from "node:path";

const DB_PATH = process.env.DB_PATH || "./data/vault.db";

async function ensureDbDir() {
  try {
    await mkdir(dirname(DB_PATH), { recursive: true });
  } catch (error) {
    console.error("Failed to create database directory:", error);
  }
}

export async function initDb(): Promise<Database> {
  await ensureDbDir();
  
  return new Promise((resolve, reject) => {
    const db = new Database(DB_PATH, async (err) => {
      if (err) {
        console.error("Database opening error:", err);
        reject(err);
        return;
      }

      db.serialize(() => {
        db.run(`
          CREATE TABLE IF NOT EXISTS passwords (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            username TEXT NOT NULL,
            password TEXT NOT NULL,
            url TEXT,
            notes TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP
          )
        `, (err) => {
          if (err) {
            console.error("Table creation error:", err);
            reject(err);
            return;
          }

          console.log("Database initialized successfully");
          resolve(db);
        });
      });
    });
  });
}

let dbInstance: Database | null = null;

export async function getDb(): Promise<Database> {
  if (!dbInstance) {
    dbInstance = await initDb();
  }
  return dbInstance;
}

export function encrypt(text: string): string {
  const key = process.env.ENCRYPTION_KEY || "default_key";
  return Buffer.from(text + key).toString("base64");
}

export function decrypt(encryptedText: string): string {
  const key = process.env.ENCRYPTION_KEY || "default_key";
  const decoded = Buffer.from(encryptedText, "base64").toString();
  return decoded.substring(0, decoded.length - key.length);
}
