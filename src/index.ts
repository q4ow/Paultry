import { Elysia } from "elysia";
import { cors } from "@elysiajs/cors";
import { swagger } from "@elysiajs/swagger";
import { passwordRoutes } from "./routes/passwords";

const PORT = process.env.PORT ? parseInt(process.env.PORT) : 3000;

const app = new Elysia()
  .use(cors())
  .use(swagger({
    documentation: {
      info: {
        title: "Password Vault API",
        version: "1.0.0",
      },
      tags: [
        { name: "passwords", description: "Password management endpoints" }
      ]
    }
  }))
  .use(passwordRoutes)
  .get("/", () => ({
    message: "Password Vault API is running",
    version: "1.0.0",
  }))
  .listen(PORT);

console.log(`🔐 Password Vault server is running at ${app.server?.hostname}:${app.server?.port}`);

export type App = typeof app;
