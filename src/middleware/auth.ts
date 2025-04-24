import { Elysia, t } from "elysia";

export const authMiddleware = new Elysia()
  .derive(({ request }) => {
    const apiKey = request.headers.get("x-api-key");
    const isAuthorized = apiKey === process.env.API_KEY;

    return {
      isAuthorized,
    };
  })
  .guard({
    beforeHandle: ({ isAuthorized, set }) => {
      if (!isAuthorized) {
        set.status = 401;
        return {
          success: false,
          error: "Unauthorized: Invalid API key",
        };
      }
    },
  });
