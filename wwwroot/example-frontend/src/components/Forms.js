import React, {
  useState
} from 'react';
import axios from 'axios';

export function Form(props) {
  const [state, setState] = useState({
    name: '',
    password: '',
    email: '',
    submitted: false
  });

  function handleChange(event) {
    setState(prevState => ({
      ...prevState,
      [event.target.name]: event.target.value
    }));
  }

  function clearForm() {
    setState({
      name: '',
      password: '',
      email: '',
      submitted: true
    });
  }

  async function handleSubmit(event) {
    event.preventDefault();
    try {
      const { data } = await axios.post('/users', JSON.stringify(state));
      props.appendUser({
        id: data.id,
        name: state.name,
        password: state.password,
        email: state.email
      });
      clearForm();
    } catch(err) {
      console.error(err);
    }
  }

  return (
    <div className="form-container">
      <form onSubmit={handleSubmit}>
        <input required type="text" value={state.name} name="name" placeholder="Name" autoComplete="username" onChange={handleChange} />
        <input required type="password" value={state.password} name="password" placeholder="Password" autoComplete="current-password" onChange={handleChange} />
        <input required type="email" value={state.email} name="email" placeholder="Email" autoComplete="current-email" onChange={handleChange} />
        <input type="submit" value="Hand Over Info" />
        {state.submitted && <p style={{color: 'red'}}>Successfully handed over your information!</p>}
      </form>
    </div>
  );
}
