import React, {
  FC,
  createContext,
  useContext,
  useState,
  useEffect,
} from "react";

export interface Session {
  isAuthenticated?: boolean;
  redirectPathOnAuthentication?: string;
  jwt?: string;
  uid?: string;
}

type SessionMutation = (session?: Session) => void;

export const initialSession: Session = {};

export const SessionContext = createContext<
  [Session, SessionMutation, SessionMutation]
>([initialSession, () => {}, () => {}]);
export const useSessionContext = () => useContext(SessionContext);

export const SessionContextProvider: FC = ({ children }) => {
  const [sessionState, setSessionState] = useState(initialSession);

  // Restore session from local storage on startup
  useEffect(() => {
    try {
      const savedSessionString = localStorage.getItem("session");
      if (savedSessionString) {
        const savedSession = JSON.parse(savedSessionString);
        if (savedSession.jwt && savedSession.uid) {
          setSessionState(savedSession);
        }
      }
    } catch (err) {
      console.error("Couldn't read or parse localStorage session", err);
    }
  }, []);

  const setSession = (session?: Session) => {
    if (session) {
      setSessionState(session);
      try {
        localStorage.setItem("session", JSON.stringify(session));
      } catch (err) {
        console.error("Writing to LocalStorage failed", err);
      }
    }
  };

  const removeSession = () => {
    setSessionState(initialSession);
    try {
      localStorage.removeItem("session");
    } catch (err) {
      console.error("Writing to LocalStorage failed", err);
    }
  };

  const defaultSessionContext: [Session, SessionMutation, SessionMutation] = [
    sessionState,
    setSession,
    removeSession,
  ];

  return (
    <SessionContext.Provider value={defaultSessionContext}>
      {children}
    </SessionContext.Provider>
  );
};
