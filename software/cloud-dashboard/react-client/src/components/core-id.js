import React, { Component, PropTypes } from 'react';
import { locMap } from "node-server/config/device-map"
import { Card, CardMedia, CardTitle, CardText, CardActions } from 'react-toolbox/lib/card';
import {handlePromiseWithData as props} from "../reducers/util";
import LoadingView from "./util/loading-view";

class CoreID extends Component {
	constructor(props) {
		super(props);
        this.state = {
			coreid: locMap[0]
		}
	}

	handleChange = value => {
		this.setState({coreid: locMap[value]});
	}

	render() {
		return (
			<div className="container">
				<Card className="container-card">
                    {
                        !props.loading &&
						<div>
							<p> Bucket Tips ({props.daysToFetch} days): </p>
							<h3>{getTotalBucketTips(props.bucketData)}</h3>
						</div>
                    }
                    {
                        props.loading &&
						<LoadingView />
                    }
				</Card>
			</div>
		)
	} 
}

export default CoreID;
