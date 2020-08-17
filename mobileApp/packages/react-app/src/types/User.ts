export interface User {
  hash: string;
  salt: string;
  userName: string;
  wateringGroups: string[];
  __v: number;
  _id: string;
}
