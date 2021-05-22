export function Table(props) {
  return (
    <table className="main-table">
      <thead>
        <tr>
          <td>Id</td>
          <td>Name</td>
          <td>Password</td>
          <td>Email</td>
        </tr>
      </thead>
      <tbody>
        {props.users.map(user => (
          <tr>
            <td>{user.id}</td>
            <td>{user.name}</td>
            <td>{user.password}</td>
            <td>{user.email}</td>
          </tr>
        ))}
      </tbody>
    </table>
  );
}
