export interface Password {
  id?: number;
  title: string;
  username: string;
  password: string;
  url?: string;
  notes?: string;
  created_at?: string;
  updated_at?: string;
}

export interface ApiResponse<T> {
  success: boolean;
  data?: T;
  error?: string;
}
