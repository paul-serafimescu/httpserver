import React, {
  useState,
  useEffect
} from 'react';
import {
  Form,
  Table
} from './components';
import axios from 'axios';

function App() {
  const [users, setUsers] = useState([]);

  useEffect(() => {
    axios.get('/users')
      .then(response => setUsers(response.data))
      .catch(error => console.error(error));
  }, []);

  function appendUser(user) {
    let users_old = [...users];
    users_old.push(user);
    setUsers(users_old);
  }

  return (
    <div className="App">
      <header>
        <h1>Example Frontend for httpserver</h1>
      </header>
      <Form appendUser={appendUser} />
      <Table users={users} />
    </div>
  );
}

export default App;
