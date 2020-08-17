import React, { useState } from "react";
import {
  IonPage,
  IonContent,
  IonInput,
  IonList,
  IonLabel,
  IonButton,
} from "@ionic/react";
import { ListItem } from "../components/List";
import Header from "../components/Header";
import Card from "../components/Card";
import styled from "styled-components";
import { useSessionContext } from "../contexts/SessionContext";
import { Redirect } from "react-router-dom";
import login from "../auth/userApi";

const StyledCard = styled(Card)`
  padding: 2rem 0;
`;

export default function Login() {
  const [usernameText, setUsernameText] = useState("");
  const [passwordText, setPasswordText] = useState("");

  const [sessionContext, setSession] = useSessionContext();

  const handleSubmit = async (event: React.FormEvent<HTMLFormElement>) => {
    event.preventDefault();
    if (usernameText !== "" && passwordText !== "") {
      try {
        const { success, user, token } = await login(
          usernameText,
          passwordText
        );
        if (success && user) {
          setSession({
            isAuthenticated: true,
            jwt: token,
            user,
            ...sessionContext,
          });
        }
      } catch (err) {
        console.error(err);
      }
    }
  };
  return !sessionContext.isAuthenticated ? (
    <IonPage>
      <Header>Login</Header>
      <IonContent>
        <form onSubmit={handleSubmit}>
          <StyledCard>
            <IonList>
              <ListItem lines="none">
                <IonLabel position="stacked">Username</IonLabel>
                <IonInput
                  required
                  name="username"
                  value={usernameText}
                  placeholder="Enter your Username"
                  onIonChange={(e) => setUsernameText(e.detail.value!)}
                />
              </ListItem>
              <ListItem lines="none">
                <IonLabel position="stacked">Password</IonLabel>
                <IonInput
                  type="password"
                  required
                  name="password"
                  value={passwordText}
                  placeholder="Enter your Password"
                  onIonChange={(e) => setPasswordText(e.detail.value!)}
                />
              </ListItem>
            </IonList>
          </StyledCard>
          <div className="ion-padding">
            <IonButton expand="block" type="submit" className="ion-no-margin">
              Login
            </IonButton>
          </div>
        </form>
      </IonContent>
    </IonPage>
  ) : (
    <Redirect to="/app" />
  );
}
