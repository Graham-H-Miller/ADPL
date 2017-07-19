import React, { Component, PropTypes } from 'react';

class CoreID extends Component {
	constructor(props) {
		super(props);
        this.state = {
			currentValue: null
		}
	}

	handleChange = value => {
		this.setState({currentValue: value});
	}

	render() {
		return (
			<div style={{maxWidth: '300px', margin:'auto'}}>
			<a>Core ID: ({this.state.coreid})</a>
			</div>
		)
	} 
}

export default CoreID;
