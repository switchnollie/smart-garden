import { User } from "../types/User";

interface UserAPIResponse {
  success: boolean;
  msg?: string;
  token?: string;
  expiresIn?: string;
  user?: User;
}

export default async function login(username: string, password: string) {
  const response: UserAPIResponse = await (
    await fetch("/api/user/login", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({
        username: username,
        password: password,
      }),
    })
  ).json();
  return response;
}
