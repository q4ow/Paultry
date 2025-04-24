import { Elysia, t } from "elysia";
import { getDb, encrypt, decrypt } from "../db";
import { Password } from "../types";
import { authMiddleware } from "../middleware/auth";

export const passwordRoutes = new Elysia()
  .use(authMiddleware)
  .group("/passwords", (app) => 
    app
      .get("/", async () => {
        const db = await getDb();
        
        return new Promise<Password[]>((resolve, reject) => {
          db.all("SELECT * FROM passwords ORDER BY updated_at DESC", (err, rows: Password[]) => {
            if (err) {
              reject(err);
              return;
            }
            
            const decryptedRows = rows.map(row => ({
              ...row,
              password: decrypt(row.password),
            }));
            
            resolve(decryptedRows);
          });
        }).then(data => ({
          success: true,
          data,
        })).catch(error => ({
          success: false,
          error: error.message,
        }));
      })
      
      .get("/:id", async ({ params }) => {
        const db = await getDb();
        
        return new Promise<Password>((resolve, reject) => {
          db.get("SELECT * FROM passwords WHERE id = ?", [params.id], (err, row: Password) => {
            if (err) {
              reject(err);
              return;
            }
            
            if (!row) {
              reject(new Error("Password not found"));
              return;
            }
            
            row.password = decrypt(row.password);
            resolve(row);
          });
        }).then(data => ({
          success: true,
          data,
        })).catch(error => ({
          success: false,
          error: error.message,
        }));
      })
      
      .post("/", 
        async ({ body }) => {
          const db = await getDb();
          const { title, username, password, url, notes } = body;
          
          const encryptedPassword = encrypt(password);
          
          return new Promise<{ id: number }>((resolve, reject) => {
            db.run(
              `INSERT INTO passwords (title, username, password, url, notes) 
               VALUES (?, ?, ?, ?, ?)`,
              [title, username, encryptedPassword, url, notes],
              function(err) {
                if (err) {
                  reject(err);
                  return;
                }
                
                resolve({ id: this.lastID });
              }
            );
          }).then(data => ({
            success: true,
            data,
          })).catch(error => ({
            success: false,
            error: error.message,
          }));
        },
        {
          body: t.Object({
            title: t.String(),
            username: t.String(),
            password: t.String(),
            url: t.Optional(t.String()),
            notes: t.Optional(t.String()),
          }),
        }
      )
      
      .put("/:id", 
        async ({ params, body }) => {
          const db = await getDb();
          const { title, username, password, url, notes } = body;
          
          const encryptedPassword = encrypt(password);
          
          return new Promise<{ success: boolean }>((resolve, reject) => {
            db.run(
              `UPDATE passwords 
               SET title = ?, username = ?, password = ?, url = ?, notes = ?, updated_at = CURRENT_TIMESTAMP
               WHERE id = ?`,
              [title, username, encryptedPassword, url, notes, params.id],
              function(err) {
                if (err) {
                  reject(err);
                  return;
                }
                
                if (this.changes === 0) {
                  reject(new Error("Password not found"));
                  return;
                }
                
                resolve({ success: true });
              }
            );
          }).then(data => ({
            success: true,
            data,
          })).catch(error => ({
            success: false,
            error: error.message,
          }));
        },
        {
          body: t.Object({
            title: t.String(),
            username: t.String(),
            password: t.String(),
            url: t.Optional(t.String()),
            notes: t.Optional(t.String()),
          }),
        }
      )
      
      .delete("/:id", async ({ params }) => {
        const db = await getDb();
        
        return new Promise<{ success: boolean }>((resolve, reject) => {
          db.run("DELETE FROM passwords WHERE id = ?", [params.id], function(err) {
            if (err) {
              reject(err);
              return;
            }
            
            if (this.changes === 0) {
              reject(new Error("Password not found"));
              return;
            }
            
            resolve({ success: true });
          });
        }).then(data => ({
          success: true,
          data,
        })).catch(error => ({
          success: false,
          error: error.message,
        }));
      })
  );

